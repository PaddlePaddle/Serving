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
import copy


class ChannelDataEcode(enum.Enum):
    OK = 0
    TIMEOUT = 1
    NOT_IMPLEMENTED = 2
    TYPE_ERROR = 3
    RPC_PACKAGE_ERROR = 4
    CLIENT_ERROR = 5
    UNKNOW = 6


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
                 error_info=None):
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
                raise ValueError("data_id and error_info cannot be None")
            datatype = ChannelDataType.ERROR.value
        else:
            if datatype == ChannelDataType.CHANNEL_NPDATA.value:
                ecode, error_info = ChannelData.check_npdata(npdata)
                if ecode != ChannelDataEcode.OK.value:
                    datatype = ChannelDataType.ERROR.value
                    logging.error(error_info)
            elif datatype == ChannelDataType.DICT.value:
                ecode, error_info = ChannelData.check_dictdata(dictdata)
                if ecode != ChannelDataEcode.OK.value:
                    datatype = ChannelDataType.ERROR.value
                    logging.error(error_info)
            else:
                raise ValueError("datatype not match")
        self.datatype = datatype
        self.npdata = npdata
        self.dictdata = dictdata
        self.id = data_id
        self.ecode = ecode
        self.error_info = error_info

    @staticmethod
    def check_dictdata(dictdata):
        ecode = ChannelDataEcode.OK.value
        error_info = None
        if not isinstance(dictdata, dict):
            ecode = ChannelDataEcode.TYPE_ERROR.value
            error_info = "the value of postped_data must " \
                        "be dict, but get {}".format(type(dictdata))
        return ecode, error_info

    @staticmethod
    def check_npdata(npdata):
        ecode = ChannelDataEcode.OK.value
        error_info = None
        for _, value in npdata.items():
            if not isinstance(value, np.ndarray):
                ecode = ChannelDataEcode.TYPE_ERROR.value
                error_info = "the value of postped_data must " \
                        "be np.ndarray, but get {}".format(type(value))
                break
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
            raise TypeError("Error type({}) in datatype.".format(self.datatype))
        return feed

    def __str__(self):
        return "type[{}], ecode[{}], id[{}]".format(
            ChannelDataType(self.datatype).name, self.ecode, self.id)


class ProcessChannel(multiprocessing.queues.Queue):
    """ 
    (Process version) The channel used for communication between Ops.

    1. Support multiple different Op feed data (multiple producer)
        Different types of data will be packaged through the data ID
    2. Support multiple different Op fetch data (multiple consumer)
        Only when all types of Ops get the data of the same ID,
        the data will be poped; The Op of the same type will not
        get the data of the same ID.
    3. (TODO) Timeout and BatchSize are not fully supported.

    Note:
    1. The ID of the data in the channel must be different.
    2. The function add_producer() and add_consumer() are not thread safe,
       and can only be called during initialization.
    """

    def __init__(self, manager, name=None, maxsize=0, timeout=None):
        # https://stackoverflow.com/questions/39496554/cannot-subclass-multiprocessing-queue-in-python-3-5/
        if sys.version_info.major == 2:
            super(ProcessChannel, self).__init__(maxsize=maxsize)
        elif sys.version_info.major == 3:
            super(ProcessChannel, self).__init__(
                maxsize=maxsize, ctx=multiprocessing.get_context())
        else:
            raise Exception("Error Python version")
        self._maxsize = maxsize
        self._timeout = timeout
        self.name = name
        self._stop = False

        self._cv = multiprocessing.Condition()

        self._producers = []
        self._producer_res_count = manager.dict()  # {data_id: count}
        self._push_res = manager.dict()  # {data_id: {op_name: data}}

        self._consumers = manager.dict()  # {op_name: idx}
        self._idx_consumer_num = manager.dict()  # {idx: num}
        self._consumer_base_idx = manager.Value('i', 0)
        self._front_res = manager.list()

    def get_producers(self):
        return self._producers

    def get_consumers(self):
        return self._consumers.keys()

    def _log(self, info_str):
        return "[{}] {}".format(self.name, info_str)

    def debug(self):
        return self._log("p: {}, c: {}".format(self.get_producers(),
                                               self.get_consumers()))

    def add_producer(self, op_name):
        """ not thread safe, and can only be called during initialization. """
        if op_name in self._producers:
            raise ValueError(
                self._log("producer({}) is already in channel".format(op_name)))
        self._producers.append(op_name)

    def add_consumer(self, op_name):
        """ not thread safe, and can only be called during initialization. """
        if op_name in self._consumers:
            raise ValueError(
                self._log("consumer({}) is already in channel".format(op_name)))
        self._consumers[op_name] = 0

        if self._idx_consumer_num.get(0) is None:
            self._idx_consumer_num[0] = 0
        self._idx_consumer_num[0] += 1

    def push(self, channeldata, op_name=None):
        logging.debug(
            self._log("{} try to push data: {}".format(op_name,
                                                       channeldata.__str__())))
        if len(self._producers) == 0:
            raise Exception(
                self._log(
                    "expected number of producers to be greater than 0, but the it is 0."
                ))
        elif len(self._producers) == 1:
            with self._cv:
                while self._stop is False:
                    try:
                        self.put(channeldata, timeout=0)
                        break
                    except Queue.Full:
                        self._cv.wait()
                logging.debug(
                    self._log("{} channel size: {}".format(op_name,
                                                           self.qsize())))
                self._cv.notify_all()
                logging.debug(self._log("{} notify all".format(op_name)))
            logging.debug(self._log("{} push data succ!".format(op_name)))
            return True
        elif op_name is None:
            raise Exception(
                self._log(
                    "There are multiple producers, so op_name cannot be None."))

        producer_num = len(self._producers)
        data_id = channeldata.id
        put_data = None
        with self._cv:
            logging.debug(self._log("{} get lock".format(op_name)))
            if data_id not in self._push_res:
                self._push_res[data_id] = {
                    name: None
                    for name in self._producers
                }
                self._producer_res_count[data_id] = 0
            # see: https://docs.python.org/3.6/library/multiprocessing.html?highlight=multiprocess#proxy-objects
            # self._push_res[data_id][op_name] = channeldata
            tmp_push_res = self._push_res[data_id]
            tmp_push_res[op_name] = channeldata
            self._push_res[data_id] = tmp_push_res

            if self._producer_res_count[data_id] + 1 == producer_num:
                put_data = self._push_res[data_id]
                self._push_res.pop(data_id)
                self._producer_res_count.pop(data_id)
            else:
                self._producer_res_count[data_id] += 1

            if put_data is None:
                logging.debug(
                    self._log("{} push data succ, but not push to queue.".
                              format(op_name)))
            else:
                while self._stop is False:
                    try:
                        logging.debug(
                            self._log("{} push data succ: {}".format(
                                op_name, put_data.__str__())))
                        self.put(put_data, timeout=0)
                        break
                    except Queue.Empty:
                        self._cv.wait()

                logging.debug(
                    self._log("multi | {} push data succ!".format(op_name)))
            self._cv.notify_all()
        return True

    def front(self, op_name=None):
        logging.debug(self._log("{} try to get data...".format(op_name)))
        if len(self._consumers) == 0:
            raise Exception(
                self._log(
                    "expected number of consumers to be greater than 0, but the it is 0."
                ))
        elif len(self._consumers) == 1:
            resp = None
            with self._cv:
                while self._stop is False and resp is None:
                    try:
                        logging.debug(
                            self._log("{} try to get(with channel empty: {})".
                                      format(op_name, self.empty())))
                        # For queue multiprocess: after putting an object on 
                        # an empty queue there may be an infinitessimal delay
                        # before the queue's :meth:`~Queue.empty`
                        # see more:
                        # - https://bugs.python.org/issue18277
                        # - https://hg.python.org/cpython/rev/860fc6a2bd21
                        resp = self.get(timeout=1e-3)
                        break
                    except Queue.Empty:
                        logging.debug(
                            self._log(
                                "{} wait for empty queue(with channel empty: {})".
                                format(op_name, self.empty())))
                        self._cv.wait()
            logging.debug(
                self._log("{} get data succ: {}".format(op_name, resp.__str__(
                ))))
            return resp
        elif op_name is None:
            raise Exception(
                self._log(
                    "There are multiple consumers, so op_name cannot be None."))

        with self._cv:
            # data_idx = consumer_idx - base_idx
            while self._stop is False and self._consumers[
                    op_name] - self._consumer_base_idx.value >= len(
                        self._front_res):
                logging.debug(
                    self._log(
                        "({}) B self._consumers: {}, self._consumer_base_idx: {}, len(self._front_res): {}".
                        format(op_name, self._consumers, self.
                               _consumer_base_idx.value, len(self._front_res))))
                try:
                    logging.debug(
                        self._log("{} try to get(with channel size: {})".format(
                            op_name, self.qsize())))
                    # For queue multiprocess: after putting an object on 
                    # an empty queue there may be an infinitessimal delay
                    # before the queue's :meth:`~Queue.empty`
                    # see more:
                    # - https://bugs.python.org/issue18277
                    # - https://hg.python.org/cpython/rev/860fc6a2bd21
                    channeldata = self.get(timeout=1e-3)
                    self._front_res.append(channeldata)
                    break
                except Queue.Empty:
                    logging.debug(
                        self._log(
                            "{} wait for empty queue(with channel size: {})".
                            format(op_name, self.qsize())))
                    self._cv.wait()

            consumer_idx = self._consumers[op_name]
            base_idx = self._consumer_base_idx.value
            data_idx = consumer_idx - base_idx
            resp = self._front_res[data_idx]
            logging.debug(self._log("{} get data: {}".format(op_name, resp)))

            self._idx_consumer_num[consumer_idx] -= 1
            if consumer_idx == base_idx and self._idx_consumer_num[
                    consumer_idx] == 0:
                self._idx_consumer_num.pop(consumer_idx)
                self._front_res.pop(0)
                self._consumer_base_idx.value += 1

            self._consumers[op_name] += 1
            new_consumer_idx = self._consumers[op_name]
            if self._idx_consumer_num.get(new_consumer_idx) is None:
                self._idx_consumer_num[new_consumer_idx] = 0
            self._idx_consumer_num[new_consumer_idx] += 1
            logging.debug(
                self._log(
                    "({}) A self._consumers: {}, self._consumer_base_idx: {}, len(self._front_res): {}".
                    format(op_name, self._consumers, self._consumer_base_idx.
                           value, len(self._front_res))))
            logging.debug(self._log("{} notify all".format(op_name)))
            self._cv.notify_all()

        logging.debug(self._log("multi | {} get data succ!".format(op_name)))
        return resp  # reference, read only

    def stop(self):
        #TODO
        self.close()
        self._stop = True
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
    3. (TODO) Timeout and BatchSize are not fully supported.

    Note:
    1. The ID of the data in the channel must be different.
    2. The function add_producer() and add_consumer() are not thread safe,
       and can only be called during initialization.
    """

    def __init__(self, name=None, maxsize=-1, timeout=None):
        Queue.Queue.__init__(self, maxsize=maxsize)
        self._maxsize = maxsize
        self._timeout = timeout
        self.name = name
        self._stop = False

        self._cv = threading.Condition()

        self._producers = []
        self._producer_res_count = {}  # {data_id: count}
        self._push_res = {}  # {data_id: {op_name: data}}

        self._consumers = {}  # {op_name: idx}
        self._idx_consumer_num = {}  # {idx: num}
        self._consumer_base_idx = 0
        self._front_res = []

    def get_producers(self):
        return self._producers

    def get_consumers(self):
        return self._consumers.keys()

    def _log(self, info_str):
        return "[{}] {}".format(self.name, info_str)

    def debug(self):
        return self._log("p: {}, c: {}".format(self.get_producers(),
                                               self.get_consumers()))

    def add_producer(self, op_name):
        """ not thread safe, and can only be called during initialization. """
        if op_name in self._producers:
            raise ValueError(
                self._log("producer({}) is already in channel".format(op_name)))
        self._producers.append(op_name)

    def add_consumer(self, op_name):
        """ not thread safe, and can only be called during initialization. """
        if op_name in self._consumers:
            raise ValueError(
                self._log("consumer({}) is already in channel".format(op_name)))
        self._consumers[op_name] = 0

        if self._idx_consumer_num.get(0) is None:
            self._idx_consumer_num[0] = 0
        self._idx_consumer_num[0] += 1

    def push(self, channeldata, op_name=None):
        logging.debug(
            self._log("{} try to push data: {}".format(op_name,
                                                       channeldata.__str__())))
        if len(self._producers) == 0:
            raise Exception(
                self._log(
                    "expected number of producers to be greater than 0, but the it is 0."
                ))
        elif len(self._producers) == 1:
            with self._cv:
                while self._stop is False:
                    try:
                        self.put(channeldata, timeout=0)
                        break
                    except Queue.Full:
                        self._cv.wait()
                self._cv.notify_all()
            logging.debug(self._log("{} push data succ!".format(op_name)))
            return True
        elif op_name is None:
            raise Exception(
                self._log(
                    "There are multiple producers, so op_name cannot be None."))

        producer_num = len(self._producers)
        data_id = channeldata.id
        put_data = None
        with self._cv:
            logging.debug(self._log("{} get lock".format(op_name)))
            if data_id not in self._push_res:
                self._push_res[data_id] = {
                    name: None
                    for name in self._producers
                }
                self._producer_res_count[data_id] = 0
            self._push_res[data_id][op_name] = channeldata
            if self._producer_res_count[data_id] + 1 == producer_num:
                put_data = self._push_res[data_id]
                self._push_res.pop(data_id)
                self._producer_res_count.pop(data_id)
            else:
                self._producer_res_count[data_id] += 1

            if put_data is None:
                logging.debug(
                    self._log("{} push data succ, but not push to queue.".
                              format(op_name)))
            else:
                while self._stop is False:
                    try:
                        self.put(put_data, timeout=0)
                        break
                    except Queue.Empty:
                        self._cv.wait()

                logging.debug(
                    self._log("multi | {} push data succ!".format(op_name)))
            self._cv.notify_all()
        return True

    def front(self, op_name=None):
        logging.debug(self._log("{} try to get data".format(op_name)))
        if len(self._consumers) == 0:
            raise Exception(
                self._log(
                    "expected number of consumers to be greater than 0, but the it is 0."
                ))
        elif len(self._consumers) == 1:
            resp = None
            with self._cv:
                while self._stop is False and resp is None:
                    try:
                        resp = self.get(timeout=0)
                        break
                    except Queue.Empty:
                        self._cv.wait()
            logging.debug(
                self._log("{} get data succ: {}".format(op_name, resp.__str__(
                ))))
            return resp
        elif op_name is None:
            raise Exception(
                self._log(
                    "There are multiple consumers, so op_name cannot be None."))

        with self._cv:
            # data_idx = consumer_idx - base_idx
            while self._stop is False and self._consumers[
                    op_name] - self._consumer_base_idx >= len(self._front_res):
                try:
                    channeldata = self.get(timeout=0)
                    self._front_res.append(channeldata)
                    break
                except Queue.Empty:
                    self._cv.wait()

            consumer_idx = self._consumers[op_name]
            base_idx = self._consumer_base_idx
            data_idx = consumer_idx - base_idx
            resp = self._front_res[data_idx]
            logging.debug(self._log("{} get data: {}".format(op_name, resp)))

            self._idx_consumer_num[consumer_idx] -= 1
            if consumer_idx == base_idx and self._idx_consumer_num[
                    consumer_idx] == 0:
                self._idx_consumer_num.pop(consumer_idx)
                self._front_res.pop(0)
                self._consumer_base_idx += 1

            self._consumers[op_name] += 1
            new_consumer_idx = self._consumers[op_name]
            if self._idx_consumer_num.get(new_consumer_idx) is None:
                self._idx_consumer_num[new_consumer_idx] = 0
            self._idx_consumer_num[new_consumer_idx] += 1

            self._cv.notify_all()

        logging.debug(self._log("multi | {} get data succ!".format(op_name)))
        # return resp  # reference, read only
        return copy.deepcopy(resp)

    def stop(self):
        #TODO
        self.close()
        self._stop = True
        self._cv.notify_all()
