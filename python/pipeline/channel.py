#   Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# pylint: disable=doc-string-missing
from time import time as _time
import threading
import multiprocessing
import multiprocessing.queues
import sys
if sys.version_info.major == 2:
    import Queue
elif sys.version_info.major == 3:
    import queue as Queue
else:
    raise Exception("Error Python version")
import numpy as np
import logging
import enum
import os
import copy

_LOGGER = logging.getLogger()


class ChannelDataEcode(enum.Enum):
    OK = 0
    TIMEOUT = 1
    NOT_IMPLEMENTED = 2
    TYPE_ERROR = 3
    RPC_PACKAGE_ERROR = 4
    CLIENT_ERROR = 5
    CLOSED_ERROR = 6
    UNKNOW = 7


class ChannelDataType(enum.Enum):
    DICT = 0
    CHANNEL_NPDATA = 1
    ERROR = 2


class ChannelData(object):
    def __init__(self,
                 datatype=None,
                 npdata=None,
                 dictdata=None,
                 data_id=None,
                 ecode=None,
                 error_info=None,
                 client_need_profile=False):
        '''
        There are several ways to use it:
        
        1. ChannelData(ChannelDataType.CHANNEL_NPDATA.value, npdata, data_id)
        2. ChannelData(ChannelDataType.DICT.value, dictdata, data_id)
        3. ChannelData(ecode, error_info, data_id)

        Protobufs are not pickle-able:
        https://stackoverflow.com/questions/55344376/how-to-import-protobuf-module
        '''
        if ecode is not None:
            if data_id is None or error_info is None:
                _LOGGER.critical("Failed to generate ChannelData: data_id"
                                 " and error_info cannot be None")
                os._exit(-1)
            datatype = ChannelDataType.ERROR.value
        else:
            if datatype == ChannelDataType.CHANNEL_NPDATA.value:
                ecode, error_info = ChannelData.check_npdata(npdata)
                if ecode != ChannelDataEcode.OK.value:
                    datatype = ChannelDataType.ERROR.value
                    _LOGGER.error("(logid={}) {}".format(data_id, error_info))
            elif datatype == ChannelDataType.DICT.value:
                ecode, error_info = ChannelData.check_dictdata(dictdata)
                if ecode != ChannelDataEcode.OK.value:
                    datatype = ChannelDataType.ERROR.value
                    _LOGGER.error("(logid={}) {}".format(data_id, error_info))
            else:
                _LOGGER.critical("(logid={}) datatype not match".format(
                    data_id))
                os._exit(-1)
        self.datatype = datatype
        self.npdata = npdata
        self.dictdata = dictdata
        self.id = data_id
        self.ecode = ecode
        self.error_info = error_info
        self.client_need_profile = client_need_profile
        self.profile_data_set = set()

    def add_profile(self, profile_set):
        if self.client_need_profile is False:
            self.client_need_profile = True
        self.profile_data_set |= profile_set

    @staticmethod
    def check_dictdata(dictdata):
        ecode = ChannelDataEcode.OK.value
        error_info = None
        if isinstance(dictdata, list):
            # batch data
            for sample in dictdata:
                if not isinstance(sample, dict):
                    ecode = ChannelDataEcode.TYPE_ERROR.value
                    error_info = "Failed to check data: the type of " \
                            "data must be dict, but get {}.".format(type(sample))
                    break
        elif not isinstance(dictdata, dict):
            # batch size = 1
            ecode = ChannelDataEcode.TYPE_ERROR.value
            error_info = "Failed to check data: the type of data must " \
                    "be dict, but get {}.".format(type(dictdata))
        return ecode, error_info

    @staticmethod
    def check_batch_npdata(batch):
        ecode = ChannelDataEcode.OK.value
        error_info = None
        for npdata in batch:
            ecode, error_info = ChannelData.check_npdata(npdata)
            if ecode != ChannelDataEcode.OK.value:
                break
        return ecode, error_info

    @staticmethod
    def check_npdata(npdata):
        ecode = ChannelDataEcode.OK.value
        error_info = None
        if isinstance(npdata, list):
            # batch data
            for sample in npdata:
                if not isinstance(sample, dict):
                    ecode = ChannelDataEcode.TYPE_ERROR.value
                    error_info = "Failed to check data: the " \
                            "value of data must be dict, but get {}.".format(
                                    type(sample))
                    break
                for _, value in sample.items():
                    if not isinstance(value, np.ndarray):
                        ecode = ChannelDataEcode.TYPE_ERROR.value
                        error_info = "Failed to check data: the" \
                                " value of data must be np.ndarray, but get {}.".format(
                                        type(value))
                        return ecode, error_info
        elif isinstance(npdata, dict):
            # batch_size = 1
            for _, value in npdata.items():
                if not isinstance(value, np.ndarray):
                    ecode = ChannelDataEcode.TYPE_ERROR.value
                    error_info = "Failed to check data: the value " \
                            "of data must be np.ndarray, but get {}.".format(
                                    type(value))
                    break
        else:
            ecode = ChannelDataEcode.TYPE_ERROR.value
            error_info = "Failed to check data: the value of data " \
                    "must be dict, but get {}.".format(type(npdata))
        return ecode, error_info

    def parse(self):
        feed = None
        if self.datatype == ChannelDataType.CHANNEL_NPDATA.value:
            # return narray
            feed = self.npdata
        elif self.datatype == ChannelDataType.DICT.value:
            # return dict
            feed = self.dictdata
        else:
            _LOGGER.critical("Failed to parse channeldata: error " \
                    "type({}) in datatype.".format(self.datatype))
            os._exit(-1)
        return feed

    def __str__(self):
        return "type[{}], ecode[{}], id[{}]".format(
            ChannelDataType(self.datatype).name, self.ecode, self.id)


class ProcessChannel(object):
    """ 
    (Process version) The channel used for communication between Ops.

    1. Support multiple different Op feed data (multiple producer)
        Different types of data will be packaged through the data ID
    2. Support multiple different Op fetch data (multiple consumer)
        Only when all types of Ops get the data of the same ID,
        the data will be poped; The Op of the same type will not
        get the data of the same ID.
    3. Function front support timeout param to make auto-batching.

    Note:
    1. The ID of the data in the channel must be different.
    2. The function add_producer() and add_consumer() are not thread safe,
       and can only be called during initialization.

    There are two buffers and one queue in Channel:

        op_A \                                           / op_D
        op_B - a. input_buf -> b. queue -> c. output_buf - op_E
        op_C /                                           \ op_F
    
    a. In input_buf, the input of multiple predecessor Ops is packed by data ID.
    b. The packed data will be stored in queue.
    c. In order to support multiple successor Ops to retrieve data, output_buf
        maintains the data obtained from queue.
    """

    def __init__(self, manager, name=None, maxsize=0):
        # For queue multiprocess: after putting an object on 
        # an empty queue there may be an infinitessimal delay
        # before the queue's :meth:`~Queue.empty`
        # see more:
        # - https://bugs.python.org/issue18277
        # - https://hg.python.org/cpython/rev/860fc6a2bd21
        self._que = manager.Queue(maxsize=maxsize)
        self._maxsize = maxsize
        self.name = name
        self._stop = manager.Value('i', 0)

        self._cv = multiprocessing.Condition()

        self._producers = []
        self._pushed_producer_count = manager.dict()  # {data_id: count}
        self._input_buf = manager.dict()  # {data_id: {op_name: data}}

        self._reset_max_cursor = 1000000000000000000
        self._consumer_cursors = manager.dict()  # {op_name: cursor}
        self._cursor_count = manager.dict()  # {cursor: count}
        self._base_cursor = manager.Value('i', 0)
        self._output_buf = manager.list()

    def get_producers(self):
        return self._producers

    def get_consumers(self):
        return self._consumer_cursors.keys()

    def _log(self, info_str):
        return "[{}] {}".format(self.name, info_str)

    def add_producer(self, op_name):
        """ not thread safe, and can only be called during initialization. """
        if op_name in self._producers:
            _LOGGER.critical(
                self._log("Failed to add producer: producer({})" \
                        " is already in channel".format(op_name)))
            os._exit(-1)
        self._producers.append(op_name)
        _LOGGER.debug(self._log("Succ add a producer: {}".format(op_name)))

    def add_consumer(self, op_name):
        """ not thread safe, and can only be called during initialization. """
        if op_name in self._consumer_cursors:
            _LOGGER.critical(
                    self._log("Failed to add consumer: consumer({})" \
                            " is already in channel".format(op_name)))
            os._exit(-1)
        self._consumer_cursors[op_name] = 0

        if self._cursor_count.get(0) is None:
            self._cursor_count[0] = 0
        self._cursor_count[0] += 1
        _LOGGER.debug(self._log("Succ add a consumer: {}".format(op_name)))

    def push(self, channeldata, op_name=None):
        _LOGGER.debug(
            self._log("(logid={}) Op({}) Pushing data".format(channeldata.id,
                                                              op_name)))
        if len(self._producers) == 0:
            _LOGGER.critical(
                self._log(
                    "(logid={}) Op({}) Failed to push data: expected number"
                    " of producers to be greater than 0, but the it is 0.".
                    format(channeldata.id, op_name)))
            os._exit(-1)
        elif len(self._producers) == 1:
            with self._cv:
                while self._stop.value == 0:
                    try:
                        self._que.put({op_name: channeldata}, timeout=0)
                        break
                    except Queue.Full:
                        self._cv.wait()
                if self._stop.value == 1:
                    raise ChannelStopError()
                self._cv.notify_all()
            _LOGGER.debug(
                self._log("(logid={}) Op({}) Pushed data into internal queue.".
                          format(channeldata.id, op_name)))
            return True
        elif op_name is None:
            _LOGGER.critical(
                self._log(
                    "(logid={}) Op({}) Failed to push data: there are multiple "
                    "producers, so op_name cannot be None.".format(
                        channeldata.id, op_name)))
            os._exit(-1)

        producer_num = len(self._producers)
        data_id = channeldata.id
        put_data = None
        with self._cv:
            if data_id not in self._input_buf:
                self._input_buf[data_id] = {
                    name: None
                    for name in self._producers
                }
                self._pushed_producer_count[data_id] = 0
            # see: https://docs.python.org/3.6/library/multiprocessing.html?highlight=multiprocess#proxy-objects
            # self._input_buf[data_id][op_name] = channeldata
            tmp_input_buf = self._input_buf[data_id]
            tmp_input_buf[op_name] = channeldata
            self._input_buf[data_id] = tmp_input_buf

            if self._pushed_producer_count[data_id] + 1 == producer_num:
                put_data = self._input_buf[data_id]
                self._input_buf.pop(data_id)
                self._pushed_producer_count.pop(data_id)
            else:
                self._pushed_producer_count[data_id] += 1

            if put_data is None:
                _LOGGER.debug(
                    self._log(
                        "(logid={}) Op({}) Pushed data into input_buffer.".
                        format(data_id, op_name)))
            else:
                while self._stop.value == 0:
                    try:
                        self._que.put(put_data, timeout=0)
                        break
                    except Queue.Empty:
                        self._cv.wait()
                if self._stop.value == 1:
                    raise ChannelStopError()

                _LOGGER.debug(
                    self._log(
                        "(logid={}) Op({}) Pushed data into internal_queue.".
                        format(data_id, op_name)))
            self._cv.notify_all()
        return True

    def front(self, op_name=None, timeout=None):
        _LOGGER.debug(
            self._log("Op({}) Getting data[?]; timeout(s)={}".format(op_name,
                                                                     timeout)))
        endtime = None
        if timeout is not None:
            if timeout <= 0:
                timeout = None
            else:
                endtime = _time() + timeout

        if len(self._consumer_cursors) == 0:
            _LOGGER.critical(
                self._log(
                    "Op({}) Failed to get data: expected number of consumers to be " \
                            "greater than 0, but the it is 0.".format(op_name)))
            os._exit(-1)
        elif len(self._consumer_cursors) == 1:
            resp = None
            with self._cv:
                while self._stop.value == 0 and resp is None:
                    try:
                        resp = self._que.get(timeout=0)
                        break
                    except Queue.Empty:
                        if timeout is not None:
                            remaining = endtime - _time()
                            if remaining <= 0.0:
                                _LOGGER.debug(
                                    self._log("Op({}) Failed to get data: "
                                              "timeout".format(op_name)))
                                raise ChannelTimeoutError()
                            self._cv.wait(remaining)
                        else:
                            self._cv.wait()
                if self._stop.value == 1:
                    raise ChannelStopError()
            _LOGGER.debug(
                self._log("(logid={}) Op({}) Got data".format(resp.values()[0]
                                                              .id, op_name)))
            return resp
        elif op_name is None:
            _LOGGER.critical(
                self._log(
                    "Op({}) Failed to get data: there are multiple consumers, "
                    "so op_name cannot be None.".format(op_name)))
            os._exit(-1)

        # In output_buf, different Ops (according to op_name) have different
        # cursors. In addition, there is a base_cursor. Their difference is
        # the data_idx to be taken by the corresponding Op at the current
        # time:    data_idx = consumer_cursor - base_cursor
        # 
        #            base_cursor    consumer_B_cursor (data_idx: 3)
        #                 |                       |
        # output_buf: | data0 | data1 | data2 | data3 |
        #                 |
        #   consumer_A_cursor (data_idx: 0)
        with self._cv:
            # When the data required by the current Op is not in output_buf,
            # it is necessary to obtain a data from queue and add it to output_buf.
            while self._stop.value == 0 and self._consumer_cursors[
                    op_name] - self._base_cursor.value >= len(self._output_buf):
                try:
                    channeldata = self._que.get(timeout=0)
                    self._output_buf.append(channeldata)
                    _LOGGER.debug(
                        self._log(
                            "(logid={}) Op({}) Pop ready item into output_buffer".
                            format(channeldata.values()[0].id, op_name)))
                    break
                except Queue.Empty:
                    if timeout is not None:
                        remaining = endtime - _time()
                        if remaining <= 0.0:
                            _LOGGER.debug(
                                self._log("Op({}) Failed to get data: timeout".
                                          format(op_name)))
                            raise ChannelTimeoutError()
                        self._cv.wait(remaining)
                    else:
                        self._cv.wait()
            if self._stop.value == 1:
                raise ChannelStopError()

            consumer_cursor = self._consumer_cursors[op_name]
            base_cursor = self._base_cursor.value
            data_idx = consumer_cursor - base_cursor
            resp = self._output_buf[data_idx]

            self._cursor_count[consumer_cursor] -= 1
            if consumer_cursor == base_cursor and self._cursor_count[
                    consumer_cursor] == 0:
                # When all the different Ops get the data that data_idx points
                # to, pop the data from output_buf.
                self._cursor_count.pop(consumer_cursor)
                self._output_buf.pop(0)
                self._base_cursor.value += 1
                # to avoid cursor overflow
                if self._base_cursor.value >= self._reset_max_cursor:
                    _LOGGER.info(self._log("Reset cursor in Channel"))
                    self._base_cursor.value -= self._reset_max_cursor
                    for name in self._consumer_cursors.keys():
                        self._consumer_cursors[name] -= self._reset_max_cursor
                    cursor_count_tmp = {
                        cursor - self._reset_max_cursor: count
                        for cursor, count in self._cursor_count.copy().items()
                    }
                    self._cursor_count.clear()
                    for cursor, count in cursor_count_tmp.items():
                        self._cursor_count[cursor] = count

            self._consumer_cursors[op_name] += 1
            new_consumer_cursor = self._consumer_cursors[op_name]
            if self._cursor_count.get(new_consumer_cursor) is None:
                self._cursor_count[new_consumer_cursor] = 0
            self._cursor_count[new_consumer_cursor] += 1

            self._cv.notify_all()

        _LOGGER.debug(
            self._log("(logid={}) Op({}) Got data from output_buffer".format(
                resp.values()[0].id, op_name)))
        return resp

    def stop(self):
        _LOGGER.info(self._log("stop."))
        self._stop.value = 1
        with self._cv:
            self._cv.notify_all()


class ThreadChannel(Queue.Queue):
    """ 
    (Thread version)The channel used for communication between Ops.

    1. Support multiple different Op feed data (multiple producer)
        Different types of data will be packaged through the data ID
    2. Support multiple different Op fetch data (multiple consumer)
        Only when all types of Ops get the data of the same ID,
        the data will be poped; The Op of the same type will not
        get the data of the same ID.
    3. Function front support timeout param to make auto-batching.

    Note:
    1. The ID of the data in the channel must be different.
    2. The function add_producer() and add_consumer() are not thread safe,
       and can only be called during initialization.

    There are two buffers and one queue in Channel:

        op_A \                                           / op_D
        op_B - a. input_buf -> b. queue -> c. output_buf - op_E
        op_C /                                           \ op_F
    
    a. In input_buf, the input of multiple predecessor Ops is packed by data ID.
    b. The packed data will be stored in queue.
    c. In order to support multiple successor Ops to retrieve data, output_buf
        maintains the data obtained from queue.
    """

    def __init__(self, name=None, maxsize=-1):
        Queue.Queue.__init__(self, maxsize=maxsize)
        self._maxsize = maxsize
        self.name = name
        self._stop = False

        self._cv = threading.Condition()

        self._producers = []
        self._pushed_producer_count = {}  # {data_id: count}
        self._input_buf = {}  # {data_id: {op_name: data}}

        self._reset_max_cursor = 1000000000000000000
        self._consumer_cursors = {}  # {op_name: idx}
        self._cursor_count = {}  # {cursor: count}
        self._base_cursor = 0
        self._output_buf = []

    def get_producers(self):
        return self._producers

    def get_consumers(self):
        return self._consumer_cursors.keys()

    def _log(self, info_str):
        return "[{}] {}".format(self.name, info_str)

    def add_producer(self, op_name):
        """ not thread safe, and can only be called during initialization. """
        if op_name in self._producers:
            _LOGGER.critical(
                self._log("Failed to add producer: producer({}) is "
                          "already in channel".format(op_name)))
            os._exit(-1)
        self._producers.append(op_name)
        _LOGGER.debug(self._log("Succ add a producer: {}".format(op_name)))

    def add_consumer(self, op_name):
        """ not thread safe, and can only be called during initialization. """
        if op_name in self._consumer_cursors:
            _LOGGER.critical(
                self._log("Failed to add consumer: consumer({}) is "
                          "already in channel".format(op_name)))
            os._exit(-1)
        self._consumer_cursors[op_name] = 0

        if self._cursor_count.get(0) is None:
            self._cursor_count[0] = 0
        self._cursor_count[0] += 1
        _LOGGER.debug(self._log("Succ add a consumer: {}".format(op_name)))

    def push(self, channeldata, op_name=None):
        _LOGGER.debug(
            self._log("(logid={}) Op({}) Pushing data".format(channeldata.id,
                                                              op_name)))
        if len(self._producers) == 0:
            _LOGGER.critical(
                self._log(
                    "(logid={}) Op({}) Failed to push data: expected number of "
                    "producers to be greater than 0, but the it is 0.".format(
                        channeldata.id, op_name)))
            os._exit(-1)
        elif len(self._producers) == 1:
            with self._cv:
                while self._stop is False:
                    try:
                        self.put({op_name: channeldata}, timeout=0)
                        break
                    except Queue.Full:
                        self._cv.wait()
                if self._stop:
                    raise ChannelStopError()
                self._cv.notify_all()
            _LOGGER.debug(
                self._log("(logid={}) Op({}) Pushed data into internal_queue.".
                          format(channeldata.id, op_name)))
            return True
        elif op_name is None:
            _LOGGER.critical(
                self._log(
                    "(logid={}) Op({}) Failed to push data: there are multiple"
                    " producers, so op_name cannot be None.".format(
                        channeldata.id, op_name)))
            os._exit(-1)

        producer_num = len(self._producers)
        data_id = channeldata.id
        put_data = None
        with self._cv:
            if data_id not in self._input_buf:
                self._input_buf[data_id] = {
                    name: None
                    for name in self._producers
                }
                self._pushed_producer_count[data_id] = 0
            self._input_buf[data_id][op_name] = channeldata
            if self._pushed_producer_count[data_id] + 1 == producer_num:
                put_data = self._input_buf[data_id]
                self._input_buf.pop(data_id)
                self._pushed_producer_count.pop(data_id)
            else:
                self._pushed_producer_count[data_id] += 1

            if put_data is None:
                _LOGGER.debug(
                    self._log(
                        "(logid={}) Op({}) Pushed data into input_buffer.".
                        format(data_id, op_name)))
            else:
                while self._stop is False:
                    try:
                        self.put(put_data, timeout=0)
                        break
                    except Queue.Empty:
                        self._cv.wait()
                if self._stop:
                    raise ChannelStopError()

                _LOGGER.debug(
                    self._log(
                        "(logid={}) Op({}) Pushed data into internal_queue.".
                        format(data_id, op_name)))
            self._cv.notify_all()
        return True

    def front(self, op_name=None, timeout=None):
        _LOGGER.debug(
            self._log("Op({}) Getting data[?]; timeout(s)={}".format(op_name,
                                                                     timeout)))
        endtime = None
        if timeout is not None:
            if timeout <= 0:
                timeout = None
            else:
                endtime = _time() + timeout

        if len(self._consumer_cursors) == 0:
            _LOGGER.critical(
                self._log(
                    "Op({}) Failed to get data: expected number of consumers to be "
                    "greater than 0, but the it is 0.".format(op_name)))
            os._exit(-1)
        elif len(self._consumer_cursors) == 1:
            resp = None
            with self._cv:
                while self._stop is False and resp is None:
                    try:
                        resp = self.get(timeout=0)
                        break
                    except Queue.Empty:
                        if timeout is not None:
                            remaining = endtime - _time()
                            if remaining <= 0.0:
                                _LOGGER.debug(
                                    self._log(
                                        "Op({}) Failed to get data: timeout".
                                        format(op_name)))
                                raise ChannelTimeoutError()
                            self._cv.wait(remaining)
                        else:
                            self._cv.wait()
                if self._stop:
                    raise ChannelStopError()
            _LOGGER.debug(
                self._log("(logid={}) Op({}) Got data".format(resp.values()[0]
                                                              .id, op_name)))
            return resp
        elif op_name is None:
            _LOGGER.critical(
                self._log("Op({}) Failed to get data: there are multiple "
                          "consumers, so op_name cannot be None.".format(
                              op_name)))
            os._exit(-1)

        # In output_buf, different Ops (according to op_name) have different
        # cursors. In addition, there is a base_cursor. Their difference is
        # the data_idx to be taken by the corresponding Op at the current
        # time:    data_idx = consumer_cursor - base_cursor
        # 
        #            base_cursor    consumer_B_cursor (data_idx: 3)
        #                 |                       |
        # output_buf: | data0 | data1 | data2 | data3 |
        #                 |
        #   consumer_A_cursor (data_idx: 0)
        with self._cv:
            # When the data required by the current Op is not in output_buf,
            # it is necessary to obtain a data from queue and add it to output_buf.
            while self._stop is False and self._consumer_cursors[
                    op_name] - self._base_cursor >= len(self._output_buf):
                try:
                    channeldata = self.get(timeout=0)
                    self._output_buf.append(channeldata)
                    _LOGGER.debug(
                        self._log(
                            "(logid={}) Op({}) Pop ready item into output_buffer".
                            format(channeldata.values()[0].id, op_name)))
                    break
                except Queue.Empty:
                    if timeout is not None:
                        remaining = endtime - _time()
                        if remaining <= 0.0:
                            _LOGGER.debug(
                                self._log("Op({}) Failed to get data: timeout".
                                          format(op_name)))
                            raise ChannelTimeoutError()
                        self._cv.wait(remaining)
                    else:
                        self._cv.wait()
            if self._stop:
                raise ChannelStopError()

            consumer_cursor = self._consumer_cursors[op_name]
            base_cursor = self._base_cursor
            data_idx = consumer_cursor - base_cursor

            resp = None

            self._cursor_count[consumer_cursor] -= 1
            if consumer_cursor == base_cursor and self._cursor_count[
                    consumer_cursor] == 0:
                # When all the different Ops get the data that data_idx points
                # to, pop the data from output_buf.
                self._cursor_count.pop(consumer_cursor)
                resp = self._output_buf.pop(0)
                self._base_cursor += 1
                # to avoid cursor overflow
                if self._base_cursor >= self._reset_max_cursor:
                    _LOGGER.info(self._log("Reset cursor in Channel"))
                    self._base_cursor -= self._reset_max_cursor
                    for name in self._consumer_cursors:
                        self._consumer_cursors[name] -= self._reset_max_cursor
                    self._cursor_count = {
                        cursor - self._reset_max_cursor: count
                        for cursor, count in self._cursor_count.items()
                    }
            else:
                resp = copy.deepcopy(self._output_buf[data_idx])

            self._consumer_cursors[op_name] += 1
            new_consumer_cursor = self._consumer_cursors[op_name]
            if self._cursor_count.get(new_consumer_cursor) is None:
                self._cursor_count[new_consumer_cursor] = 0
            self._cursor_count[new_consumer_cursor] += 1

            self._cv.notify_all()

        _LOGGER.debug(
            self._log("(logid={}) Op({}) Got data from output_buffer".format(
                resp.values()[0].id, op_name)))
        return resp

    def stop(self):
        _LOGGER.info(self._log("stop."))
        self._stop = True
        with self._cv:
            self._cv.notify_all()


class ChannelTimeoutError(RuntimeError):
    def __init__(self):
        pass


class ChannelStopError(RuntimeError):
    def __init__(self):
        pass
