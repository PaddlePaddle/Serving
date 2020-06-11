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
import Queue
import os
import sys
import paddle_serving_server
from paddle_serving_client import MultiLangClient as Client
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


class ChannelDataType(enum.Enum):
    CHANNEL_PBDATA = 0
    CHANNEL_FUTURE = 1


class ChannelData(object):
    def __init__(self,
                 future=None,
                 pbdata=None,
                 data_id=None,
                 callback_func=None):
        self.future = future
        if pbdata is None:
            if data_id is None:
                raise ValueError("data_id cannot be None")
            pbdata = channel_pb2.ChannelData()
            pbdata.type = ChannelDataType.CHANNEL_FUTURE.value
            pbdata.ecode = ChannelDataEcode.OK.value
            pbdata.id = data_id
        self.pbdata = pbdata
        self.callback_func = callback_func

    def parse(self):
        # return narray
        feed = {}
        if self.pbdata.type == ChannelDataType.CHANNEL_PBDATA.value:
            for inst in self.pbdata.insts:
                feed[inst.name] = np.frombuffer(inst.data, dtype=inst.type)
                feed[inst.name].shape = np.frombuffer(inst.shape, dtype="int32")
        elif self.pbdata.type == ChannelDataType.CHANNEL_FUTURE.value:
            feed = self.future.result()
            if self.callback_func is not None:
                feed = self.callback_func(feed)
        else:
            raise TypeError(
                self._log("Error type({}) in pbdata.type.".format(
                    self.pbdata.type)))
        return feed


class Channel(Queue.Queue):
    """ 
    The channel used for communication between Ops.

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
                                                       channeldata.pbdata)))
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
                    except Queue.Empty:
                        self._cv.wait()
                self._cv.notify_all()
            logging.debug(self._log("{} push data succ!".format(op_name)))
            return True
        elif op_name is None:
            raise Exception(
                self._log(
                    "There are multiple producers, so op_name cannot be None."))

        producer_num = len(self._producers)
        data_id = channeldata.pbdata.id
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
            logging.debug(self._log("{} get data succ!".format(op_name)))
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
        return resp  # reference, read only

    def stop(self):
        #TODO
        self.close()
        self._stop = True


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
        self._run = False
        self.name = name  # to identify the type of OP, it must be globally unique
        self._concurrency = concurrency  # amount of concurrency
        self.set_input_ops(inputs)
        self.set_client(client_config, server_name, fetch_names)
        self._server_model = server_model
        self._server_port = server_port
        self._device = device
        self._timeout = timeout
        self._retry = retry
        self._input = None
        self._outputs = []

    def set_client(self, client_config, server_name, fetch_names):
        self._client = None
        if client_config is None or \
                server_name is None or \
                fetch_names is None:
            return
        self._client = Client()
        self._client.load_client_config(client_config)
        self._client.connect([server_name])
        self._fetch_names = fetch_names

    def with_serving(self):
        return self._client is not None

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
        if not isinstance(channel, Channel):
            raise TypeError(
                self._log('input channel must be Channel type, not {}'.format(
                    type(channel))))
        channel.add_consumer(self.name)
        self._input = channel

    def get_output_channels(self):
        return self._outputs

    def add_output_channel(self, channel):
        if not isinstance(channel, Channel):
            raise TypeError(
                self._log('output channel must be Channel type, not {}'.format(
                    type(channel))))
        channel.add_producer(self.name)
        self._outputs.append(channel)

    def preprocess(self, channeldata):
        if isinstance(channeldata, dict):
            raise Exception(
                self._log(
                    'this Op has multiple previous inputs. Please override this method'
                ))
        feed = channeldata.parse()
        return feed

    def midprocess(self, data):
        if not isinstance(data, dict):
            raise Exception(
                self._log(
                    'data must be dict type(the output of preprocess()), but get {}'.
                    format(type(data))))
        logging.debug(self._log('data: {}'.format(data)))
        logging.debug(self._log('fetch: {}'.format(self._fetch_names)))
        call_future = self._client.predict(
            feed=data, fetch=self._fetch_names, asyn=True)
        logging.debug(self._log("get call_future"))
        return call_future

    def postprocess(self, output_data):
        return output_data

    def errorprocess(self, error_info, data_id):
        data = channel_pb2.ChannelData()
        data.ecode = 1
        data.id = data_id
        data.error_info = error_info
        return data

    def stop(self):
        self._input.stop()
        for channel in self._outputs:
            channel.stop()
        self._run = False

    def _parse_channeldata(self, channeldata):
        data_id, error_data = None, None
        if isinstance(channeldata, dict):
            parsed_data = {}
            key = channeldata.keys()[0]
            data_id = channeldata[key].pbdata.id
            for _, data in channeldata.items():
                if data.pbdata.ecode != 0:
                    error_data = data
                    break
        else:
            data_id = channeldata.pbdata.id
            if channeldata.pbdata.ecode != 0:
                error_data = channeldata.pbdata
        return data_id, error_data

    def start(self, concurrency_idx):
        self._run = True
        while self._run:
            _profiler.record("{}{}-get_0".format(self.name, concurrency_idx))
            input_data = self._input.front(self.name)
            _profiler.record("{}{}-get_1".format(self.name, concurrency_idx))
            logging.debug(self._log("input_data: {}".format(input_data)))

            data_id, error_data = self._parse_channeldata(input_data)

            output_data = None
            if error_data is None:
                _profiler.record("{}{}-prep_0".format(self.name,
                                                      concurrency_idx))
                data = self.preprocess(input_data)
                _profiler.record("{}{}-prep_1".format(self.name,
                                                      concurrency_idx))

                call_future = None
                error_info = None
                if self.with_serving():
                    _profiler.record("{}{}-midp_0".format(self.name,
                                                          concurrency_idx))
                    if self._timeout > 0:
                        for i in range(self._retry):
                            try:
                                call_future = func_timeout.func_timeout(
                                    self._timeout,
                                    self.midprocess,
                                    args=(data, ))
                            except func_timeout.FunctionTimedOut:
                                logging.error("error: timeout")
                                error_info = "{}({}): timeout".format(
                                    self.name, concurrency_idx)
                                if i + 1 < self._retry:
                                    error_info = None
                                    logging.warn(
                                        self._log("warn: timeout, retry({})".
                                                  format(i + 1)))
                            except Exception as e:
                                logging.error("error: {}".format(e))
                                error_info = "{}({}): {}".format(
                                    self.name, concurrency_idx, e)
                                logging.warn(self._log(e))
                                # TODO
                                break
                            else:
                                break
                    else:
                        call_future = self.midprocess(data)

                    _profiler.record("{}{}-midp_1".format(self.name,
                                                          concurrency_idx))
                _profiler.record("{}{}-postp_0".format(self.name,
                                                       concurrency_idx))
                if error_info is not None:
                    error_data = self.errorprocess(error_info, data_id)
                    output_data = ChannelData(pbdata=error_data)
                else:
                    if self.with_serving():  # use call_future
                        output_data = ChannelData(
                            future=call_future,
                            data_id=data_id,
                            callback_func=self.postprocess)
                    else:
                        post_data = self.postprocess(data)
                        if not isinstance(post_data, dict):
                            raise TypeError(
                                self._log(
                                    'output_data must be dict type, but get {}'.
                                    format(type(output_data))))
                        pbdata = channel_pb2.ChannelData()
                        for name, value in post_data.items():
                            inst = channel_pb2.Inst()
                            inst.data = value.tobytes()
                            inst.name = name
                            inst.shape = np.array(
                                value.shape, dtype="int32").tobytes()
                            inst.type = str(value.dtype)
                            pbdata.insts.append(inst)
                        pbdata.ecode = 0
                        pbdata.id = data_id
                        output_data = ChannelData(pbdata=pbdata)
                _profiler.record("{}{}-postp_1".format(self.name,
                                                       concurrency_idx))
            else:
                output_data = ChannelData(pbdata=error_data)

            _profiler.record("{}{}-push_0".format(self.name, concurrency_idx))
            for channel in self._outputs:
                channel.push(output_data, self.name)
            _profiler.record("{}{}-push_1".format(self.name, concurrency_idx))

    def _log(self, info_str):
        return "[{}] {}".format(self.name, info_str)

    def get_concurrency(self):
        return self._concurrency


class GeneralPythonService(
        general_python_service_pb2_grpc.GeneralPythonService):
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

    def _log(self, info_str):
        return "[{}] {}".format(self.name, info_str)

    def set_in_channel(self, in_channel):
        if not isinstance(in_channel, Channel):
            raise TypeError(
                self._log('in_channel must be Channel type, but get {}'.format(
                    type(in_channel))))
        in_channel.add_producer(self.name)
        self._in_channel = in_channel

    def set_out_channel(self, out_channel):
        if not isinstance(out_channel, Channel):
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
                data_id = channeldata.pbdata.id
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
        pbdata = channel_pb2.ChannelData()
        data_id = self._get_next_id()
        pbdata.id = data_id
        for idx, name in enumerate(request.feed_var_names):
            logging.debug(
                self._log('name: {}'.format(request.feed_var_names[idx])))
            logging.debug(self._log('data: {}'.format(request.feed_insts[idx])))
            inst = channel_pb2.Inst()
            inst.data = request.feed_insts[idx]
            inst.shape = request.shape[idx]
            inst.name = name
            inst.type = request.type[idx]
            pbdata.insts.append(inst)
        pbdata.ecode = 0  #TODO: parse request error
        return ChannelData(pbdata=pbdata), data_id

    def _pack_data_for_resp(self, channeldata):
        logging.debug(self._log('get channeldata'))
        logging.debug(self._log('gen resp'))
        resp = pyservice_pb2.Response()
        resp.ecode = channeldata.pbdata.ecode
        if resp.ecode == 0:
            if channeldata.pbdata.type == ChannelDataType.CHANNEL_PBDATA.value:
                for inst in channeldata.pbdata.insts:
                    logging.debug(self._log('append data'))
                    resp.fetch_insts.append(inst.data)
                    logging.debug(self._log('append name'))
                    resp.fetch_var_names.append(inst.name)
                    logging.debug(self._log('append shape'))
                    resp.shape.append(inst.shape)
                    logging.debug(self._log('append type'))
                    resp.type.append(inst.type)
            elif channeldata.pbdata.type == ChannelDataType.CHANNEL_FUTURE.value:
                feed = channeldata.futures.result()
                if channeldata.callback_func is not None:
                    feed = channeldata.callback_func(feed)
                for name, var in feed:
                    logging.debug(self._log('append data'))
                    resp.fetch_insts.append(var.tobytes())
                    logging.debug(self._log('append name'))
                    resp.fetch_var_names.append(name)
                    logging.debug(self._log('append shape'))
                    resp.shape.append(
                        np.array(
                            var.shape, dtype="int32").tobytes())
                    resp.type.append(str(var.dtype))
            else:
                raise TypeError(
                    self._log("Error type({}) in pbdata.type.".format(
                        self.pbdata.type)))
        else:
            resp.error_info = channeldata.pbdata.error_info
        return resp

    def inference(self, request, context):
        _profiler.record("{}-prepack_0".format(self.name))
        data, data_id = self._pack_data_for_infer(request)
        _profiler.record("{}-prepack_1".format(self.name))

        resp_channeldata = None
        for i in range(self._retry):
            logging.debug(self._log('push data'))
            _profiler.record("{}-push_0".format(self.name))
            self._in_channel.push(data, self.name)
            _profiler.record("{}-push_1".format(self.name))

            logging.debug(self._log('wait for infer'))
            _profiler.record("{}-fetch_0".format(self.name))
            resp_channeldata = self._get_data_in_globel_resp_dict(data_id)
            _profiler.record("{}-fetch_1".format(self.name))

            if resp_channeldata.pbdata.ecode == 0:
                break
            logging.warn("retry({}): {}".format(
                i + 1, resp_channeldata.pbdata.error_info))

        _profiler.record("{}-postpack_0".format(self.name))
        resp = self._pack_data_for_resp(resp_channeldata)
        _profiler.record("{}-postpack_1".format(self.name))
        _profiler.print_profile()
        return resp


class PyServer(object):
    def __init__(self, retry=2, profile=False):
        self._channels = []
        self._user_ops = []
        self._total_ops = []
        self._op_threads = []
        self._port = None
        self._worker_num = None
        self._in_channel = None
        self._out_channel = None
        self._retry = retry
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
        outdegs = {op.name: [] for op in self._user_ops}
        que_idx = 0  # scroll queue 
        ques = [Queue.Queue() for _ in range(2)]
        for idx, op in enumerate(self._user_ops):
            # check the name of op is globally unique
            if op.name in indeg_num:
                raise Exception("the name of Op must be unique")
            indeg_num[op.name] = len(op.get_input_ops())
            if indeg_num[op.name] == 0:
                ques[que_idx].put(op)
            for pred_op in op.get_input_ops():
                outdegs[pred_op.name].append(op)

        # get dag_views
        dag_views = []
        sorted_op_num = 0
        while True:
            que = ques[que_idx]
            next_que = ques[(que_idx + 1) % 2]
            dag_view = []
            while que.qsize() != 0:
                op = que.get()
                dag_view.append(op)
                op_name = op.name
                sorted_op_num += 1
                for succ_op in outdegs[op_name]:
                    indeg_num[succ_op.name] -= 1
                    if indeg_num[succ_op.name] == 0:
                        next_que.put(succ_op)
            dag_views.append(dag_view)
            if next_que.qsize() == 0:
                break
            que_idx = (que_idx + 1) % 2
        if sorted_op_num < len(self._user_ops):
            raise Exception("not legal DAG")
        if len(dag_views[0]) != 1:
            raise Exception("DAG contains multiple input Ops")
        if len(dag_views[-1]) != 1:
            raise Exception("DAG contains multiple output Ops")

        # create channels and virtual ops
        virtual_op_idx = 0
        channel_idx = 0
        virtual_ops = []
        channels = []
        input_channel = None
        for v_idx, view in enumerate(dag_views):
            if v_idx + 1 >= len(dag_views):
                break
            next_view = dag_views[v_idx + 1]
            actual_next_view = []
            pred_op_of_next_view_op = {}
            for op in view:
                # create virtual op
                for succ_op in outdegs[op.name]:
                    if succ_op in next_view:
                        actual_next_view.append(succ_op)
                        if succ_op.name not in pred_op_of_next_view_op:
                            pred_op_of_next_view_op[succ_op.name] = []
                        pred_op_of_next_view_op[succ_op.name].append(op)
                    else:
                        vop = Op(name="vir{}".format(virtual_op_idx), inputs=[])
                        virtual_op_idx += 1
                        virtual_ops.append(virtual_op)
                        outdegs[vop.name] = [succ_op]
                        actual_next_view.append(vop)
                        # TODO: combine vop
                        pred_op_of_next_view_op[vop.name] = [op]
            # create channel
            processed_op = set()
            for o_idx, op in enumerate(actual_next_view):
                op_name = op.name
                if op_name in processed_op:
                    continue
                channel = Channel(name="chl{}".format(channel_idx))
                channel_idx += 1
                channels.append(channel)
                op.add_input_channel(channel)
                pred_ops = pred_op_of_next_view_op[op_name]
                if v_idx == 0:
                    input_channel = channel
                else:
                    for pred_op in pred_ops:
                        pred_op.add_output_channel(channel)
                processed_op.add(op_name)
                # combine channel
                for other_op in actual_next_view[o_idx:]:
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
                        other_op.add_input_channel(channel)
                        processed_op.add(other_op.name)
        output_channel = Channel(name="Ochl")
        channels.append(output_channel)
        last_op = dag_views[-1][0]
        last_op.add_output_channel(output_channel)

        self._ops = virtual_ops
        for op in self._user_ops:
            if len(op.get_input_ops()) == 0:
                continue
            self._ops.append(op)
        self._channels = channels
        return input_channel, output_channel

    def prepare_server(self, port, worker_num):
        self._port = port
        self._worker_num = worker_num

        input_channel, output_channel = self._topo_sort()
        self._in_channel = input_channel
        self._out_channel = output_channel
        for op in self._ops:
            if op.with_serving():
                self.prepare_serving(op)
        self.gen_desc()

    def _op_start_wrapper(self, op, concurrency_idx):
        return op.start(concurrency_idx)

    def _run_ops(self):
        for op in self._ops:
            op_concurrency = op.get_concurrency()
            logging.debug("run op: {}, op_concurrency: {}".format(
                op.name, op_concurrency))
            for c in range(op_concurrency):
                th = threading.Thread(
                    target=self._op_start_wrapper, args=(op, c))
                th.start()
                self._op_threads.append(th)

    def _stop_ops(self):
        for op in self._ops:
            op.stop()

    def run_server(self):
        self._run_ops()
        server = grpc.server(
            futures.ThreadPoolExecutor(max_workers=self._worker_num))
        general_python_service_pb2_grpc.add_GeneralPythonServiceServicer_to_server(
            GeneralPythonService(self._in_channel, self._out_channel,
                                 self._retry), server)
        server.add_insecure_port('[::]:{}'.format(self._port))
        server.start()
        server.wait_for_termination()
        self._stop_ops()  # TODO
        for th in self._op_threads:
            th.join()

    def prepare_serving(self, op):
        model_path = op._server_model
        port = op._server_port
        device = op._device

        if device == "cpu":
            cmd = "(Use MultiLangServer) python -m paddle_serving_server.serve" \
                  " --model {} --thread 4 --port {} --use_multilang &>/dev/null &".format(model_path, port)
        else:
            cmd = "(Use MultiLangServer) python -m paddle_serving_server_gpu.serve" \
                  " --model {} --thread 4 --port {} --use_multilang &>/dev/null &".format(model_path, port)
        # run a server (not in PyServing)
        logging.info("run a server (not in PyServing): {}".format(cmd))
        return
        # os.system(cmd)
