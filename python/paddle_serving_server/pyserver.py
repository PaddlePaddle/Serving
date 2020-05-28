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
from paddle_serving_client import Client
from concurrent import futures
import numpy as np
import grpc
import general_python_service_pb2
import general_python_service_pb2_grpc
import python_service_channel_pb2
import logging
import random
import time
import func_timeout


class _TimeProfiler(object):
    def __init__(self):
        self._pid = os.getpid()
        self._print_head = 'PROFILE\tpid:{}\t'.format(self._pid)
        self._time_record = Queue.Queue()
        self._enable = False

    def enable(self, enable):
        self._enable = enable

    def record(self, name_with_tag):
        name_with_tag = name_with_tag.split("_")
        tag = name_with_tag[-1]
        name = '_'.join(name_with_tag[:-1])
        self._time_record.put((name, tag, int(round(time.time() * 1000000))))

    def print_profile(self):
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
        self._name = name

        self._cv = threading.Condition()

        self._producers = []
        self._producer_res_count = {}  # {data_id: count}
        self._push_res = {}  # {data_id: {op_name: data}}

        self._front_wait_interval = 0.1  # second
        self._consumers = {}  # {op_name: idx}
        self._idx_consumer_num = {}  # {idx: num}
        self._consumer_base_idx = 0
        self._front_res = []

    def get_producers(self):
        return self._producers

    def get_consumers(self):
        return self._consumers.keys()

    def _log(self, info_str):
        return "[{}] {}".format(self._name, info_str)

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

    def push(self, data, op_name=None):
        logging.debug(
            self._log("{} try to push data: {}".format(op_name, data)))
        if len(self._producers) == 0:
            raise Exception(
                self._log(
                    "expected number of producers to be greater than 0, but the it is 0."
                ))
        elif len(self._producers) == 1:
            with self._cv:
                while True:
                    try:
                        self.put(data, timeout=0)
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
        data_id = data.id
        put_data = None
        with self._cv:
            logging.debug(self._log("{} get lock ~".format(op_name)))
            if data_id not in self._push_res:
                self._push_res[data_id] = {
                    name: None
                    for name in self._producers
                }
                self._producer_res_count[data_id] = 0
            self._push_res[data_id][op_name] = data
            if self._producer_res_count[data_id] + 1 == producer_num:
                put_data = self._push_res[data_id]
                self._push_res.pop(data_id)
                self._producer_res_count.pop(data_id)
            else:
                self._producer_res_count[data_id] += 1

            if put_data is None:
                logging.debug(
                    self._log("{} push data succ, not not push to queue.".
                              format(op_name)))
            else:
                while True:
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
                while resp is None:
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
            while self._consumers[op_name] - self._consumer_base_idx >= len(
                    self._front_res):
                try:
                    data = self.get(timeout=0)
                    self._front_res.append(data)
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


class Op(object):
    def __init__(self,
                 name,
                 input,
                 in_dtype,
                 outputs,
                 out_dtype,
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
        # TODO: globally unique check
        self._name = name  # to identify the type of OP, it must be globally unique
        self._concurrency = concurrency  # amount of concurrency
        self.set_input(input)
        self._in_dtype = in_dtype
        self.set_outputs(outputs)
        self._out_dtype = out_dtype
        self._client = None
        if client_config is not None and \
                server_name is not None and \
                fetch_names is not None:
            self.set_client(client_config, server_name, fetch_names)
        self._server_model = server_model
        self._server_port = server_port
        self._device = device
        self._timeout = timeout
        self._retry = retry

    def set_client(self, client_config, server_name, fetch_names):
        self._client = Client()
        self._client.load_client_config(client_config)
        self._client.connect([server_name])
        self._fetch_names = fetch_names

    def with_serving(self):
        return self._client is not None

    def get_input(self):
        return self._input

    def set_input(self, channel):
        if not isinstance(channel, Channel):
            raise TypeError(
                self._log('input channel must be Channel type, not {}'.format(
                    type(channel))))
        channel.add_consumer(self._name)
        self._input = channel

    def get_outputs(self):
        return self._outputs

    def set_outputs(self, channels):
        if not isinstance(channels, list):
            raise TypeError(
                self._log('output channels must be list type, not {}'.format(
                    type(channels))))
        for channel in channels:
            channel.add_producer(self._name)
        self._outputs = channels

    def preprocess(self, data):
        if isinstance(data, dict):
            raise Exception(
                self._log(
                    'this Op has multiple previous inputs. Please override this method'
                ))
        feed = {}
        for inst in data.insts:
            feed[inst.name] = np.frombuffer(inst.data, dtype=self._in_dtype)
        return feed

    def midprocess(self, data):
        if not isinstance(data, dict):
            raise Exception(
                self._log(
                    'data must be dict type(the output of preprocess()), but get {}'.
                    format(type(data))))
        logging.debug(self._log('data: {}'.format(data)))
        logging.debug(self._log('fetch: {}'.format(self._fetch_names)))
        fetch_map = self._client.predict(feed=data, fetch=self._fetch_names)
        logging.debug(self._log("finish predict"))
        return fetch_map

    def postprocess(self, output_data):
        raise Exception(
            self._log(
                'Please override this method to convert data to the format in channel.'
            ))

    def errorprocess(self, error_info):
        data = python_service_channel_pb2.ChannelData()
        data.is_error = 1
        data.error_info = error_info
        return data

    def stop(self):
        self._run = False

    def start(self, concurrency_idx):
        self._run = True
        while self._run:
            _profiler.record("{}{}-get_0".format(self._name, concurrency_idx))
            input_data = self._input.front(self._name)
            _profiler.record("{}{}-get_1".format(self._name, concurrency_idx))
            data_id = None
            output_data = None
            error_data = None
            logging.debug(self._log("input_data: {}".format(input_data)))
            if isinstance(input_data, dict):
                key = input_data.keys()[0]
                data_id = input_data[key].id
                for _, data in input_data.items():
                    if data.is_error != 0:
                        error_data = data
                        break
            else:
                data_id = input_data.id
                if input_data.is_error != 0:
                    error_data = input_data

            if error_data is None:
                _profiler.record("{}{}-prep_0".format(self._name,
                                                      concurrency_idx))
                data = self.preprocess(input_data)
                _profiler.record("{}{}-prep_1".format(self._name,
                                                      concurrency_idx))

                error_info = None
                if self.with_serving():
                    for i in range(self._retry):
                        _profiler.record("{}{}-midp_0".format(self._name,
                                                              concurrency_idx))
                        if self._time > 0:
                            try:
                                middata = func_timeout.func_timeout(
                                    self._time, self.midprocess, args=(data, ))
                            except func_timeout.FunctionTimedOut:
                                logging.error("error: timeout")
                                error_info = "{}({}): timeout".format(
                                    self._name, concurrency_idx)
                            except Exception as e:
                                logging.error("error: {}".format(e))
                                error_info = "{}({}): {}".format(
                                    self._name, concurrency_idx, e)
                        else:
                            middata = self.midprocess(data)
                        _profiler.record("{}{}-midp_1".format(self._name,
                                                              concurrency_idx))
                        if error_info is None:
                            data = middata
                            break
                        if i + 1 < self._retry:
                            error_info = None
                            logging.warn(
                                self._log("warn: timeout, retry({})".format(i +
                                                                            1)))

                _profiler.record("{}{}-postp_0".format(self._name,
                                                       concurrency_idx))
                if error_info is not None:
                    output_data = self.errorprocess(error_info)
                else:
                    output_data = self.postprocess(data)

                    if not isinstance(output_data,
                                      python_service_channel_pb2.ChannelData):
                        raise TypeError(
                            self._log(
                                'output_data must be ChannelData type, but get {}'.
                                format(type(output_data))))
                    output_data.is_error = 0
                _profiler.record("{}{}-postp_1".format(self._name,
                                                       concurrency_idx))

                output_data.id = data_id
            else:
                output_data = error_data

            _profiler.record("{}{}-push_0".format(self._name, concurrency_idx))
            for channel in self._outputs:
                channel.push(output_data, self._name)
            _profiler.record("{}{}-push_1".format(self._name, concurrency_idx))

    def _log(self, info_str):
        return "[{}] {}".format(self._name, info_str)

    def get_concurrency(self):
        return self._concurrency


class GeneralPythonService(
        general_python_service_pb2_grpc.GeneralPythonService):
    def __init__(self, in_channel, out_channel, retry=2):
        super(GeneralPythonService, self).__init__()
        self._name = "#G"
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
        return "[{}] {}".format(self._name, info_str)

    def set_in_channel(self, in_channel):
        if not isinstance(in_channel, Channel):
            raise TypeError(
                self._log('in_channel must be Channel type, but get {}'.format(
                    type(in_channel))))
        in_channel.add_producer(self._name)
        self._in_channel = in_channel

    def set_out_channel(self, out_channel):
        if not isinstance(out_channel, Channel):
            raise TypeError(
                self._log('out_channel must be Channel type, but get {}'.format(
                    type(out_channel))))
        out_channel.add_consumer(self._name)
        self._out_channel = out_channel

    def _recive_out_channel_func(self):
        while True:
            data = self._out_channel.front(self._name)
            if not isinstance(data, python_service_channel_pb2.ChannelData):
                raise TypeError(
                    self._log('data must be ChannelData type, but get {}'.
                              format(type(data))))
            with self._cv:
                self._globel_resp_dict[data.id] = data
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
        data = python_service_channel_pb2.ChannelData()
        data_id = self._get_next_id()
        data.id = data_id
        for idx, name in enumerate(request.feed_var_names):
            logging.debug(
                self._log('name: {}'.format(request.feed_var_names[idx])))
            logging.debug(self._log('data: {}'.format(request.feed_insts[idx])))
            inst = python_service_channel_pb2.Inst()
            inst.data = request.feed_insts[idx]
            inst.name = name
            data.insts.append(inst)
        return data, data_id

    def _pack_data_for_resp(self, data):
        logging.debug(self._log('get data'))
        resp = general_python_service_pb2.Response()
        logging.debug(self._log('gen resp'))
        logging.debug(data)
        resp.is_error = data.is_error
        if resp.is_error == 0:
            for inst in data.insts:
                logging.debug(self._log('append data'))
                resp.fetch_insts.append(inst.data)
                logging.debug(self._log('append name'))
                resp.fetch_var_names.append(inst.name)
        else:
            resp.error_info = data.error_info
        return resp

    def inference(self, request, context):
        _profiler.record("{}-prepack_0".format(self._name))
        data, data_id = self._pack_data_for_infer(request)
        _profiler.record("{}-prepack_1".format(self._name))

        for i in range(self._retry):
            logging.debug(self._log('push data'))
            _profiler.record("{}-push_0".format(self._name))
            self._in_channel.push(data, self._name)
            _profiler.record("{}-push_1".format(self._name))

            logging.debug(self._log('wait for infer'))
            resp_data = None
            _profiler.record("{}-fetch_0".format(self._name))
            resp_data = self._get_data_in_globel_resp_dict(data_id)
            _profiler.record("{}-fetch_1".format(self._name))

            if resp_data.is_error == 0:
                break
            logging.warn("retry({}): {}".format(i + 1, resp_data.error_info))

        _profiler.record("{}-postpack_0".format(self._name))
        resp = self._pack_data_for_resp(resp_data)
        _profiler.record("{}-postpack_1".format(self._name))
        _profiler.print_profile()
        return resp


class PyServer(object):
    def __init__(self, retry=2, profile=False):
        self._channels = []
        self._ops = []
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
        self._ops.append(op)

    def gen_desc(self):
        logging.info('here will generate desc for paas')
        pass

    def prepare_server(self, port, worker_num):
        self._port = port
        self._worker_num = worker_num
        inputs = set()
        outputs = set()
        for op in self._ops:
            inputs |= set([op.get_input()])
            outputs |= set(op.get_outputs())
            if op.with_serving():
                self.prepare_serving(op)
        in_channel = inputs - outputs
        out_channel = outputs - inputs
        if len(in_channel) != 1 or len(out_channel) != 1:
            raise Exception(
                "in_channel(out_channel) more than 1 or no in_channel(out_channel)"
            )
        self._in_channel = in_channel.pop()
        self._out_channel = out_channel.pop()
        self.gen_desc()

    def _op_start_wrapper(self, op, concurrency_idx):
        return op.start(concurrency_idx)

    def _run_ops(self):
        for op in self._ops:
            op_concurrency = op.get_concurrency()
            logging.debug("run op: {}, op_concurrency: {}".format(
                op._name, op_concurrency))
            for c in range(op_concurrency):
                # th = multiprocessing.Process(
                th = threading.Thread(
                    target=self._op_start_wrapper, args=(op, c))
                th.start()
                self._op_threads.append(th)

    def run_server(self):
        self._run_ops()
        server = grpc.server(
            futures.ThreadPoolExecutor(max_workers=self._worker_num))
        general_python_service_pb2_grpc.add_GeneralPythonServiceServicer_to_server(
            GeneralPythonService(self._in_channel, self._out_channel,
                                 self._retry), server)
        server.add_insecure_port('[::]:{}'.format(self._port))
        server.start()
        try:
            for th in self._op_threads:
                th.join()
            server.join()
        except KeyboardInterrupt:
            server.stop(0)

    def prepare_serving(self, op):
        model_path = op._server_model
        port = op._server_port
        device = op._device

        if device == "cpu":
            cmd = "python -m paddle_serving_server.serve --model {} --thread 4 --port {} &>/dev/null &".format(
                model_path, port)
        else:
            cmd = "python -m paddle_serving_server_gpu.serve --model {} --thread 4 --port {} &>/dev/null &".format(
                model_path, port)
        # run a server (not in PyServing)
        logging.info("run a server (not in PyServing): {}".format(cmd))
        return
        # os.system(cmd)
