# Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
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
import os
from paddle_serving_client import MultiLangClient, Client
from concurrent import futures
import numpy as np
import grpc
from .proto import general_model_config_pb2 as m_config
from .proto import general_python_service_pb2 as pyservice_pb2
from .proto import pyserving_channel_pb2 as channel_pb2
from .proto import general_python_service_pb2_grpc
import logging
import random
import time
import func_timeout
import enum
import collections
import copy


class _TimeProfiler(object):
    def __init__(self):
        self._pid = os.getpid()
        self._print_head = 'PROFILE\tpid:{}\t'.format(self._pid)
        self._time_record = Queue.Queue()
        self._enable = False

    def enable(self, enable):
        self._enable = enable

    def record(self, name_with_tag):
        if self._enable is False:
            return
        name_with_tag = name_with_tag.split("_")
        tag = name_with_tag[-1]
        name = '_'.join(name_with_tag[:-1])
        self._time_record.put((name, tag, int(round(time.time() * 1000000))))

    def print_profile(self):
        if self._enable is False:
            return
        sys.stderr.write(self._print_head)
        tmp = {}
        while not self._time_record.empty():
            name, tag, timestamp = self._time_record.get()
            if name in tmp:
                ptag, ptimestamp = tmp.pop(name)
                sys.stderr.write("{}_{}:{} ".format(name, ptag, ptimestamp))
                sys.stderr.write("{}_{}:{} ".format(name, tag, timestamp))
            else:
                tmp[name] = (tag, timestamp)
        sys.stderr.write('\n')
        for name, item in tmp.items():
            tag, timestamp = item
            self._time_record.put((name, tag, timestamp))


_profiler = _TimeProfiler()


class ChannelDataEcode(enum.Enum):
    OK = 0
    TIMEOUT = 1
    NOT_IMPLEMENTED = 2
    TYPE_ERROR = 3
    RPC_PACKAGE_ERROR = 4
    UNKNOW = 5


class ChannelDataType(enum.Enum):
    CHANNEL_PBDATA = 0
    CHANNEL_FUTURE = 1
    CHANNEL_NPDATA = 2
    ERROR = 3


class ChannelData(object):
    def __init__(self,
                 datatype=None,
                 future=None,
                 pbdata=None,
                 npdata=None,
                 data_id=None,
                 callback_func=None,
                 ecode=None,
                 error_info=None):
        '''
        There are several ways to use it:
        
        1. ChannelData(ChannelDataType.CHANNEL_FUTURE.value, future, pbdata[, callback_func])
        2. ChannelData(ChannelDataType.CHANNEL_FUTURE.value, future, data_id[, callback_func])
        3. ChannelData(ChannelDataType.CHANNEL_PBDATA.value, pbdata)
        4. ChannelData(ChannelDataType.CHANNEL_PBDATA.value, npdata, data_id)
        5. ChannelData(ChannelDataType.CHANNEL_NPDATA.value, npdata, data_id)
        6. ChannelData(ecode, error_info, data_id)

        Protobufs are not pickle-able:
        https://stackoverflow.com/questions/55344376/how-to-import-protobuf-module
        '''
        if ecode is not None:
            if data_id is None or error_info is None:
                raise ValueError("data_id and error_info cannot be None")
            datatype = ChannelDataType.ERROR.value
        else:
            if datatype == ChannelDataType.CHANNEL_FUTURE.value:
                if data_id is None:
                    raise ValueError("data_id cannot be None")
                ecode = ChannelDataEcode.OK.value
            elif datatype == ChannelDataType.CHANNEL_PBDATA.value:
                if pbdata is None:
                    if data_id is None:
                        raise ValueError("data_id cannot be None")
                    pbdata = channel_pb2.ChannelData()
                    ecode, error_info = self._check_npdata(npdata)
                    if ecode != ChannelDataEcode.OK.value:
                        logging.error(error_info)
                    else:
                        for name, value in npdata.items():
                            inst = channel_pb2.Inst()
                            inst.data = value.tobytes()
                            inst.name = name
                            inst.shape = np.array(
                                value.shape, dtype="int32").tobytes()
                            inst.type = str(value.dtype)
                            pbdata.insts.append(inst)
            elif datatype == ChannelDataType.CHANNEL_NPDATA.value:
                ecode, error_info = self._check_npdata(npdata)
                if ecode != ChannelDataEcode.OK.value:
                    logging.error(error_info)
            else:
                raise ValueError("datatype not match")
        self.future = future
        self.pbdata = pbdata
        self.npdata = npdata
        self.datatype = datatype
        self.id = data_id
        self.ecode = ecode
        self.error_info = error_info
        self.callback_func = callback_func

    def _check_npdata(self, npdata):
        ecode = ChannelDataEcode.OK.value
        error_info = None
        for _, value in npdata.items():
            if not isinstance(value, np.ndarray):
                ecode = ChannelDataEcode.TYPE_ERROR.value
                error_info = log("the value of postped_data must " \
                        "be np.ndarray, but get {}".format(type(value)))
                break
        return ecode, error_info

    def parse(self):
        # return narray
        feed = None
        if self.datatype == ChannelDataType.CHANNEL_PBDATA.value:
            feed = {}
            for inst in self.pbdata.insts:
                feed[inst.name] = np.frombuffer(inst.data, dtype=inst.type)
                feed[inst.name].shape = np.frombuffer(inst.shape, dtype="int32")
        elif self.datatype == ChannelDataType.CHANNEL_FUTURE.value:
            feed = self.future.result()
            if self.callback_func is not None:
                feed = self.callback_func(feed)
        elif self.datatype == ChannelDataType.CHANNEL_NPDATA.value:
            feed = self.npdata
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
        return copy.deepcopy(resp)  # reference, read only

    def stop(self):
        #TODO
        self.close()
        self._stop = True
        self._cv.notify_all()


class Op(object):
    def __init__(self,
                 name,
                 inputs,
                 server_model=None,
                 server_port=None,
                 device=None,
                 client_config=None,
                 server_name=None,
                 fetch_names=None,
                 concurrency=1,
                 timeout=-1,
                 retry=2):
        self._is_run = False
        self.name = name  # to identify the type of OP, it must be globally unique
        self._concurrency = concurrency  # amount of concurrency
        self.set_input_ops(inputs)
        self.with_serving = False
        self._client_config = client_config
        self._server_name = server_name
        self._fetch_names = fetch_names
        self._server_model = server_model
        self._server_port = server_port
        self._device = device
        if self._client_config is not None and \
                self._server_name is not None and \
                self._fetch_names is not None:
            self.with_serving = True
        self._timeout = timeout
        self._retry = max(1, retry)
        self._input = None
        self._outputs = []

    def init_client(self, client_type, client_config, server_name, fetch_names):
        if self.with_serving == False:
            logging.debug("{} no client".format(self.name))
            return
        logging.debug("{} client_config: {}".format(self.name, client_config))
        logging.debug("{} server_name: {}".format(self.name, server_name))
        logging.debug("{} fetch_names: {}".format(self.name, fetch_names))
        if client_type == 'brpc':
            self._client = Client()
        elif client_type == 'grpc':
            self._client = MultiLangClient()
        else:
            raise ValueError("unknow client type: {}".format(client_type))
        self._client.load_client_config(client_config)
        self._client.connect([server_name])
        self._fetch_names = fetch_names

    def get_input_channel(self):
        return self._input

    def get_input_ops(self):
        return self._input_ops

    def set_input_ops(self, ops):
        if not isinstance(ops, list):
            ops = [] if ops is None else [ops]
        self._input_ops = []
        for op in ops:
            if not isinstance(op, Op):
                raise TypeError(
                    self._log('input op must be Op type, not {}'.format(
                        type(op))))
            self._input_ops.append(op)

    def add_input_channel(self, channel):
        if not isinstance(channel, (ThreadChannel, ProcessChannel)):
            raise TypeError(
                self._log('input channel must be Channel type, not {}'.format(
                    type(channel))))
        channel.add_consumer(self.name)
        self._input = channel

    def get_output_channels(self):
        return self._outputs

    def add_output_channel(self, channel):
        if not isinstance(channel, (ThreadChannel, ProcessChannel)):
            raise TypeError(
                self._log('output channel must be Channel type, not {}'.format(
                    type(channel))))
        channel.add_producer(self.name)
        self._outputs.append(channel)

    def preprocess(self, channeldata):
        if isinstance(channeldata, dict):
            raise NotImplementedError(
                'this Op has multiple previous inputs. Please override this method'
            )
        feed = channeldata.parse()
        return feed

    def midprocess(self, data, use_future=True):
        if not isinstance(data, dict):
            raise Exception(
                self._log(
                    'data must be dict type(the output of preprocess()), but get {}'.
                    format(type(data))))
        logging.debug(self._log('data: {}'.format(data)))
        logging.debug(self._log('fetch: {}'.format(self._fetch_names)))
        if isinstance(self._client, MultiLangClient):
            call_result = self._client.predict(
                feed=data, fetch=self._fetch_names, asyn=use_future)
        else:
            call_result = self._client.predict(
                feed=data, fetch=self._fetch_names)
        logging.debug(self._log("get call_result"))
        return call_result

    def postprocess(self, output_data):
        return output_data

    def stop(self):
        self._input.stop()
        for channel in self._outputs:
            channel.stop()
        self._is_run = False

    def _parse_channeldata(self, channeldata):
        data_id, error_channeldata = None, None
        if isinstance(channeldata, dict):
            parsed_data = {}
            key = list(channeldata.keys())[0]
            data_id = channeldata[key].id
            for _, data in channeldata.items():
                if data.ecode != ChannelDataEcode.OK.value:
                    error_channeldata = data
                    break
        else:
            data_id = channeldata.id
            if channeldata.ecode != ChannelDataEcode.OK.value:
                error_channeldata = channeldata
        return data_id, error_channeldata

    def _push_to_output_channels(self, data, channels, name=None):
        if name is None:
            name = self.name
        for channel in channels:
            channel.push(data, name)

    def start_with_process(self, client_type, use_future):
        proces = []
        for concurrency_idx in range(self._concurrency):
            p = multiprocessing.Process(
                target=self._run,
                args=(concurrency_idx, self.get_input_channel(),
                      self.get_output_channels(), client_type, use_future))
            p.start()
            proces.append(p)
        return proces

    def start_with_thread(self, client_type, use_future):
        threads = []
        for concurrency_idx in range(self._concurrency):
            t = threading.Thread(
                target=self._run,
                args=(concurrency_idx, self.get_input_channel(),
                      self.get_output_channels(), client_type, use_future))
            t.start()
            threads.append(t)
        return threads

    def _run(self, concurrency_idx, input_channel, output_channels, client_type,
             use_future):
        # create client based on client_type
        self.init_client(client_type, self._client_config, self._server_name,
                         self._fetch_names)
        op_info_prefix = "[{}|{}]".format(self.name, concurrency_idx)
        log = self._get_log_func(op_info_prefix)
        self._is_run = True
        tid = threading.current_thread().ident
        while self._is_run:
            _profiler.record("{}-get#{}_0".format(op_info_prefix, tid))
            channeldata = input_channel.front(self.name)
            _profiler.record("{}-get#{}_1".format(op_info_prefix, tid))
            logging.debug(log("input_data: {}".format(channeldata)))

            data_id, error_channeldata = self._parse_channeldata(channeldata)

            # error data in predecessor Op
            if error_channeldata is not None:
                self._push_to_output_channels(error_channeldata,
                                              output_channels)
                continue

            # preprecess
            try:
                _profiler.record("{}-prep#{}_0".format(op_info_prefix, tid))
                preped_data = self.preprocess(channeldata)
                _profiler.record("{}-prep#{}_1".format(op_info_prefix, tid))
            except NotImplementedError as e:
                # preprocess function not implemented
                error_info = log(e)
                logging.error(error_info)
                self._push_to_output_channels(
                    ChannelData(
                        ecode=ChannelDataEcode.NOT_IMPLEMENTED.value,
                        error_info=error_info,
                        data_id=data_id),
                    output_channels)
                continue
            except TypeError as e:
                # Error type in channeldata.datatype
                error_info = log(e)
                logging.error(error_info)
                self._push_to_output_channels(
                    ChannelData(
                        ecode=ChannelDataEcode.TYPE_ERROR.value,
                        error_info=error_info,
                        data_id=data_id),
                    output_channels)
                continue
            except Exception as e:
                error_info = log(e)
                logging.error(error_info)
                self._push_to_output_channels(
                    ChannelData(
                        ecode=ChannelDataEcode.UNKNOW.value,
                        error_info=error_info,
                        data_id=data_id),
                    output_channels)
                continue

            # midprocess
            midped_data = None
            if self.with_serving:
                ecode = ChannelDataEcode.OK.value
                _profiler.record("{}-midp#{}_0".format(op_info_prefix, tid))
                if self._timeout <= 0:
                    try:
                        midped_data = self.midprocess(preped_data, use_future)
                    except Exception as e:
                        ecode = ChannelDataEcode.UNKNOW.value
                        error_info = log(e)
                        logging.error(error_info)
                else:
                    for i in range(self._retry):
                        try:
                            midped_data = func_timeout.func_timeout(
                                self._timeout,
                                self.midprocess,
                                args=(preped_data, use_future))
                        except func_timeout.FunctionTimedOut as e:
                            if i + 1 >= self._retry:
                                ecode = ChannelDataEcode.TIMEOUT.value
                                error_info = log(e)
                                logging.error(error_info)
                            else:
                                logging.warn(
                                    log("timeout, retry({})".format(i + 1)))
                        except Exception as e:
                            ecode = ChannelDataEcode.UNKNOW.value
                            error_info = log(e)
                            logging.error(error_info)
                            break
                        else:
                            break
                if ecode != ChannelDataEcode.OK.value:
                    self._push_to_output_channels(
                        ChannelData(
                            ecode=ecode, error_info=error_info,
                            data_id=data_id),
                        output_channels)
                    continue
                _profiler.record("{}-midp#{}_1".format(op_info_prefix, tid))
            else:
                midped_data = preped_data

            # postprocess
            output_data = None
            _profiler.record("{}-postp#{}_0".format(op_info_prefix, tid))
            if self.with_serving and client_type == 'grpc' and use_future:
                # use call_future
                output_data = ChannelData(
                    datatype=ChannelDataType.CHANNEL_FUTURE.value,
                    future=midped_data,
                    data_id=data_id,
                    callback_func=self.postprocess)
            else:
                try:
                    postped_data = self.postprocess(midped_data)
                except Exception as e:
                    ecode = ChannelDataEcode.UNKNOW.value
                    error_info = log(e)
                    logging.error(error_info)
                    self._push_to_output_channels(
                        ChannelData(
                            ecode=ecode, error_info=error_info,
                            data_id=data_id),
                        output_channels)
                    continue
                if not isinstance(postped_data, dict):
                    ecode = ChannelDataEcode.TYPE_ERROR.value
                    error_info = log("output of postprocess funticon must be " \
                            "dict type, but get {}".format(type(postped_data)))
                    logging.error(error_info)
                    self._push_to_output_channels(
                        ChannelData(
                            ecode=ecode, error_info=error_info,
                            data_id=data_id),
                        output_channels)
                    continue

                output_data = ChannelData(
                    ChannelDataType.CHANNEL_NPDATA.value,
                    npdata=postped_data,
                    data_id=data_id)
            _profiler.record("{}-postp#{}_1".format(op_info_prefix, tid))

            # push data to channel (if run succ)
            _profiler.record("{}-push#{}_0".format(op_info_prefix, tid))
            self._push_to_output_channels(output_data, output_channels)
            _profiler.record("{}-push#{}_1".format(op_info_prefix, tid))

    def _log(self, info):
        return "{} {}".format(self.name, info)

    def _get_log_func(self, op_info_prefix):
        def log_func(info_str):
            return "{} {}".format(op_info_prefix, info_str)

        return log_func

    def get_concurrency(self):
        return self._concurrency


class VirtualOp(Op):
    ''' For connecting two channels. '''

    def __init__(self, name, concurrency=1):
        super(VirtualOp, self).__init__(
            name=name, inputs=None, concurrency=concurrency)
        self._virtual_pred_ops = []

    def add_virtual_pred_op(self, op):
        self._virtual_pred_ops.append(op)

    def add_output_channel(self, channel):
        if not isinstance(channel, (ThreadChannel, ProcessChannel)):
            raise TypeError(
                self._log('output channel must be Channel type, not {}'.format(
                    type(channel))))
        for op in self._virtual_pred_ops:
            channel.add_producer(op.name)
        self._outputs.append(channel)

    def _run_with_brpc(self, concurrency_idx, input_channel, output_channels):
        op_info_prefix = "[{}|{}]".format(self.name, concurrency_idx)
        log = self._get_log_func(op_info_prefix)
        self._is_run = True
        tid = threading.current_thread().ident
        while self._is_run:
            _profiler.record("{}-get#{}_0".format(op_info_prefix, tid))
            channeldata = input_channel.front(self.name)
            _profiler.record("{}-get#{}_1".format(op_info_prefix, tid))

            _profiler.record("{}-push#{}_0".format(op_info_prefix, tid))
            if isinstance(channeldata, dict):
                for name, data in channeldata.items():
                    self._push_to_output_channels(
                        data, channels=output_channels, name=name)
            else:
                self._push_to_output_channels(
                    channeldata,
                    channels=output_channels,
                    name=self._virtual_pred_ops[0].name)
            _profiler.record("{}-push#{}_1".format(op_info_prefix, tid))


class GeneralPythonService(
        general_python_service_pb2_grpc.GeneralPythonServiceServicer):
    def __init__(self, in_channel, out_channel, retry=2):
        super(GeneralPythonService, self).__init__()
        self.name = "#G"
        self.set_in_channel(in_channel)
        self.set_out_channel(out_channel)
        logging.debug(self._log(in_channel.debug()))
        logging.debug(self._log(out_channel.debug()))
        #TODO: 
        #  multi-lock for different clients
        #  diffenert lock for server and client
        self._id_lock = threading.Lock()
        self._cv = threading.Condition()
        self._globel_resp_dict = {}
        self._id_counter = 0
        self._retry = retry
        self._recive_func = threading.Thread(
            target=GeneralPythonService._recive_out_channel_func, args=(self, ))
        self._recive_func.start()
        self._tid = threading.current_thread().ident

    def _log(self, info_str):
        return "[{}] {}".format(self.name, info_str)

    def set_in_channel(self, in_channel):
        if not isinstance(in_channel, (ThreadChannel, ProcessChannel)):
            raise TypeError(
                self._log('in_channel must be Channel type, but get {}'.format(
                    type(in_channel))))
        in_channel.add_producer(self.name)
        self._in_channel = in_channel

    def set_out_channel(self, out_channel):
        if not isinstance(out_channel, (ThreadChannel, ProcessChannel)):
            raise TypeError(
                self._log('out_channel must be Channel type, but get {}'.format(
                    type(out_channel))))
        out_channel.add_consumer(self.name)
        self._out_channel = out_channel

    def _recive_out_channel_func(self):
        while True:
            channeldata = self._out_channel.front(self.name)
            if not isinstance(channeldata, ChannelData):
                raise TypeError(
                    self._log('data must be ChannelData type, but get {}'.
                              format(type(channeldata))))
            with self._cv:
                data_id = channeldata.id
                self._globel_resp_dict[data_id] = channeldata
                self._cv.notify_all()

    def _get_next_id(self):
        with self._id_lock:
            self._id_counter += 1
            return self._id_counter - 1

    def _get_data_in_globel_resp_dict(self, data_id):
        resp = None
        with self._cv:
            while data_id not in self._globel_resp_dict:
                self._cv.wait()
            resp = self._globel_resp_dict.pop(data_id)
            self._cv.notify_all()
        return resp

    def _pack_data_for_infer(self, request):
        logging.debug(self._log('start inferce'))
        data_id = self._get_next_id()
        npdata = {}
        try:
            for idx, name in enumerate(request.feed_var_names):
                logging.debug(
                    self._log('name: {}'.format(request.feed_var_names[idx])))
                logging.debug(
                    self._log('data: {}'.format(request.feed_insts[idx])))
                npdata[name] = np.frombuffer(
                    request.feed_insts[idx], dtype=request.type[idx])
                npdata[name].shape = np.frombuffer(
                    request.shape[idx], dtype="int32")
        except Exception as e:
            return ChannelData(
                ecode=ChannelDataEcode.RPC_PACKAGE_ERROR.value,
                error_info="rpc package error",
                data_id=data_id), data_id
        else:
            return ChannelData(
                datatype=ChannelDataType.CHANNEL_NPDATA.value,
                npdata=npdata,
                data_id=data_id), data_id

    def _pack_data_for_resp(self, channeldata):
        logging.debug(self._log('get channeldata'))
        resp = pyservice_pb2.Response()
        resp.ecode = channeldata.ecode
        if resp.ecode == ChannelDataEcode.OK.value:
            if channeldata.datatype == ChannelDataType.CHANNEL_PBDATA.value:
                for inst in channeldata.pbdata.insts:
                    resp.fetch_insts.append(inst.data)
                    resp.fetch_var_names.append(inst.name)
                    resp.shape.append(inst.shape)
                    resp.type.append(inst.type)
            elif channeldata.datatype in (ChannelDataType.CHANNEL_FUTURE.value,
                                          ChannelDataType.CHANNEL_NPDATA.value):
                feed = channeldata.parse()
                for name, var in feed.items():
                    resp.fetch_insts.append(var.tobytes())
                    resp.fetch_var_names.append(name)
                    resp.shape.append(
                        np.array(
                            var.shape, dtype="int32").tobytes())
                    resp.type.append(str(var.dtype))
            else:
                raise TypeError(
                    self._log("Error type({}) in datatype.".format(
                        channeldata.datatype)))
        else:
            resp.error_info = channeldata.error_info
        return resp

    def inference(self, request, context):
        _profiler.record("{}-prepack#{}_0".format(self.name, self._tid))
        data, data_id = self._pack_data_for_infer(request)
        _profiler.record("{}-prepack#{}_1".format(self.name, self._tid))

        resp_channeldata = None
        for i in range(self._retry):
            logging.debug(self._log('push data'))
            _profiler.record("{}-push#{}_0".format(self.name, self._tid))
            self._in_channel.push(data, self.name)
            _profiler.record("{}-push#{}_1".format(self.name, self._tid))

            logging.debug(self._log('wait for infer'))
            _profiler.record("{}-fetch#{}_0".format(self.name, self._tid))
            resp_channeldata = self._get_data_in_globel_resp_dict(data_id)
            _profiler.record("{}-fetch#{}_1".format(self.name, self._tid))

            if resp_channeldata.ecode == ChannelDataEcode.OK.value:
                break
            if i + 1 < self._retry:
                logging.warn("retry({}): {}".format(
                    i + 1, resp_channeldata.error_info))

        _profiler.record("{}-postpack#{}_0".format(self.name, self._tid))
        resp = self._pack_data_for_resp(resp_channeldata)
        _profiler.record("{}-postpack#{}_1".format(self.name, self._tid))
        _profiler.print_profile()
        return resp


class PyServer(object):
    def __init__(self,
                 use_multithread=True,
                 client_type='brpc',
                 use_future=False,
                 retry=2,
                 profile=False):
        self._channels = []
        self._user_ops = []
        self._actual_ops = []
        self._port = None
        self._worker_num = None
        self._in_channel = None
        self._out_channel = None
        self._retry = retry
        self._use_multithread = use_multithread
        self._client_type = client_type
        self._use_future = use_future
        if not self._use_multithread:
            self._manager = multiprocessing.Manager()
            if profile:
                raise Exception(
                    "profile cannot be used in multiprocess version temporarily")
            if self._use_future:
                raise Exception("cannot use future in multiprocess")
        if self._client_type == 'brpc' and self._use_future:
            logging.warn("brpc impl cannot use future")
        _profiler.enable(profile)

    def add_channel(self, channel):
        self._channels.append(channel)

    def add_op(self, op):
        self._user_ops.append(op)

    def add_ops(self, ops):
        self._user_ops.extend(ops)

    def gen_desc(self):
        logging.info('here will generate desc for PAAS')
        pass

    def _topo_sort(self):
        indeg_num = {}
        que_idx = 0  # scroll queue 
        ques = [Queue.Queue() for _ in range(2)]
        for op in self._user_ops:
            if len(op.get_input_ops()) == 0:
                op.name = "#G"  # update read_op.name
                break
        outdegs = {op.name: [] for op in self._user_ops}
        zero_indeg_num, zero_outdeg_num = 0, 0
        for idx, op in enumerate(self._user_ops):
            # check the name of op is globally unique
            if op.name in indeg_num:
                raise Exception("the name of Op must be unique")
            indeg_num[op.name] = len(op.get_input_ops())
            if indeg_num[op.name] == 0:
                ques[que_idx].put(op)
                zero_indeg_num += 1
            for pred_op in op.get_input_ops():
                outdegs[pred_op.name].append(op)
        if zero_indeg_num != 1:
            raise Exception("DAG contains multiple input Ops")
        for _, succ_list in outdegs.items():
            if len(succ_list) == 0:
                zero_outdeg_num += 1
        if zero_outdeg_num != 1:
            raise Exception("DAG contains multiple output Ops")

        # topo sort to get dag_views
        dag_views = []
        sorted_op_num = 0
        while True:
            que = ques[que_idx]
            next_que = ques[(que_idx + 1) % 2]
            dag_view = []
            while que.qsize() != 0:
                op = que.get()
                dag_view.append(op)
                sorted_op_num += 1
                for succ_op in outdegs[op.name]:
                    indeg_num[succ_op.name] -= 1
                    if indeg_num[succ_op.name] == 0:
                        next_que.put(succ_op)
            dag_views.append(dag_view)
            if next_que.qsize() == 0:
                break
            que_idx = (que_idx + 1) % 2
        if sorted_op_num < len(self._user_ops):
            raise Exception("not legal DAG")

        # create channels and virtual ops
        def name_generator(prefix):
            def number_generator():
                idx = 0
                while True:
                    yield "{}{}".format(prefix, idx)
                    idx += 1

            return number_generator()

        def gen_channel(name_gen):
            channel = None
            if self._use_multithread:
                if sys.version_info.major == 2:
                    channel = ThreadChannel(name=name_gen.next())
                elif sys.version_info.major == 3:
                    channel = ThreadChannel(name=name_gen.__next__())
                else:
                    raise Exception("Error Python version")
            else:
                if sys.version_info.major == 2:
                    channel = ProcessChannel(
                        self._manager, name=name_gen.next())
                elif sys.version_info.major == 3:
                    channel = ProcessChannel(
                        self._manager, name=name_gen.__next__())
                else:
                    raise Exception("Error Python version")
            return channel

        def gen_virtual_op(name_gen):
            virtual_op = None
            if sys.version_info.major == 2:
                virtual_op = VirtualOp(name=name_gen.next())
            elif sys.version_info.major == 3:
                virtual_op = VirtualOp(name=op_name_gen.__next__())
            else:
                raise Exception("Error Python version")
            return virtual_op

        virtual_op_name_gen = name_generator("vir")
        channel_name_gen = name_generator("chl")
        virtual_ops = []
        channels = []
        input_channel = None
        actual_view = None
        for v_idx, view in enumerate(dag_views):
            if v_idx + 1 >= len(dag_views):
                break
            next_view = dag_views[v_idx + 1]
            if actual_view is None:
                actual_view = view
            actual_next_view = []
            pred_op_of_next_view_op = {}
            for op in actual_view:
                # find actual succ op in next view and create virtual op
                for succ_op in outdegs[op.name]:
                    if succ_op in next_view:
                        if succ_op not in actual_next_view:
                            actual_next_view.append(succ_op)
                        if succ_op.name not in pred_op_of_next_view_op:
                            pred_op_of_next_view_op[succ_op.name] = []
                        pred_op_of_next_view_op[succ_op.name].append(op)
                    else:
                        # create virtual op
                        virtual_op = gen_virtual_op(virtual_op_name_gen)
                        virtual_ops.append(virtual_op)
                        outdegs[virtual_op.name] = [succ_op]
                        actual_next_view.append(virtual_op)
                        pred_op_of_next_view_op[virtual_op.name] = [op]
                        virtual_op.add_virtual_pred_op(op)
            actual_view = actual_next_view
            # create channel
            processed_op = set()
            for o_idx, op in enumerate(actual_next_view):
                if op.name in processed_op:
                    continue
                channel = gen_channel(channel_name_gen)
                channels.append(channel)
                logging.debug("{} => {}".format(channel.name, op.name))
                op.add_input_channel(channel)
                pred_ops = pred_op_of_next_view_op[op.name]
                if v_idx == 0:
                    input_channel = channel
                else:
                    # if pred_op is virtual op, it will use ancestors as producers to channel
                    for pred_op in pred_ops:
                        logging.debug("{} => {}".format(pred_op.name,
                                                        channel.name))
                        pred_op.add_output_channel(channel)
                processed_op.add(op.name)
                # find same input op to combine channel
                for other_op in actual_next_view[o_idx + 1:]:
                    if other_op.name in processed_op:
                        continue
                    other_pred_ops = pred_op_of_next_view_op[other_op.name]
                    if len(other_pred_ops) != len(pred_ops):
                        continue
                    same_flag = True
                    for pred_op in pred_ops:
                        if pred_op not in other_pred_ops:
                            same_flag = False
                            break
                    if same_flag:
                        logging.debug("{} => {}".format(channel.name,
                                                        other_op.name))
                        other_op.add_input_channel(channel)
                        processed_op.add(other_op.name)
        output_channel = gen_channel(channel_name_gen)
        channels.append(output_channel)
        last_op = dag_views[-1][0]
        last_op.add_output_channel(output_channel)

        self._actual_ops = virtual_ops
        for op in self._user_ops:
            if len(op.get_input_ops()) == 0:
                # pass read op
                continue
            self._actual_ops.append(op)
        self._channels = channels
        for c in channels:
            logging.debug(c.debug())
        return input_channel, output_channel

    def prepare_server(self, port, worker_num):
        self._port = port
        self._worker_num = worker_num

        input_channel, output_channel = self._topo_sort()
        self._in_channel = input_channel
        self._out_channel = output_channel
        for op in self._actual_ops:
            if op.with_serving:
                self.prepare_serving(op)
        self.gen_desc()

    def _run_ops(self):
        threads_or_proces = []
        for op in self._actual_ops:
            if self._use_multithread:
                threads_or_proces.extend(
                    op.start_with_thread(self._client_type, self._use_future))
            else:
                threads_or_proces.extend(
                    op.start_with_process(self._client_type, self._use_future))
        return threads_or_proces

    def _stop_ops(self):
        for op in self._actual_ops:
            op.stop()

    def run_server(self):
        op_threads_or_proces = self._run_ops()
        server = grpc.server(
            futures.ThreadPoolExecutor(max_workers=self._worker_num))
        general_python_service_pb2_grpc.add_GeneralPythonServiceServicer_to_server(
            GeneralPythonService(self._in_channel, self._out_channel,
                                 self._retry), server)
        server.add_insecure_port('[::]:{}'.format(self._port))
        server.start()
        server.wait_for_termination()
        self._stop_ops()  # TODO
        for x in op_threads_or_proces:
            x.join()

    def prepare_serving(self, op):
        model_path = op._server_model
        port = op._server_port
        device = op._device

        if self._client_type == "grpc":
            if device == "cpu":
                cmd = "(Use grpc impl) python -m paddle_serving_server.serve" \
                      " --model {} --thread 4 --port {} --use_multilang &>/dev/null &".format(model_path, port)
            else:
                cmd = "(Use grpc impl) python -m paddle_serving_server_gpu.serve" \
                      " --model {} --thread 4 --port {} --use_multilang &>/dev/null &".format(model_path, port)
        elif self._client_type == "brpc":
            if device == "cpu":
                cmd = "(Use brpc impl) python -m paddle_serving_server.serve" \
                      " --model {} --thread 4 --port {} &>/dev/null &".format(model_path, port)
            else:
                cmd = "(Use brpc impl) python -m paddle_serving_server_gpu.serve" \
                      " --model {} --thread 4 --port {} &>/dev/null &".format(model_path, port)
        else:
            raise Exception("unknow client type: {}".format(self._client_type))
        # run a server (not in PyServing)
        logging.info("run a server (not in PyServing): {}".format(cmd))
