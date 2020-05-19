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
import paddle_serving_server
from paddle_serving_client import Client
from concurrent import futures
import numpy as np
import grpc
import general_python_service_pb2
import general_python_service_pb2_grpc
import python_service_channel_pb2
import logging
import time


class Channel(Queue.Queue):
    def __init__(self, maxsize=-1, timeout=None, batchsize=1):
        Queue.Queue.__init__(self, maxsize=maxsize)
        self._maxsize = maxsize
        self._timeout = timeout
        self._batchsize = batchsize
        self._pushlock = threading.Lock()
        self._frontlock = threading.Lock()
        self._pushbatch = []

        self._consumer = {}  # {op_name: idx}
        self._consumer_base_idx = 0
        self._frontbatch = []
        self._idx_consumer_num = {}

    def add_consumer(self, op_name):
        """ not thread safe """
        if op_name in self._consumer:
            raise ValueError("op_name({}) is already in channel".format(
                op_name))
        self._consumer_id[op_name] = 0

        if self._idx_consumer_num.get(0) is None:
            self._idx_consumer_num[0] = 0
        self._idx_consumer_num[0] += 1

    def push(self, item):
        with self._pushlock:
            self._pushbatch.append(item)
            if len(self._pushbatch) == self._batchsize:
                self.put(self._pushbatch, timeout=self._timeout)
                self._pushbatch = []

    def front(self, op_name):
        if len(self._consumer) == 0:
            raise Exception(
                "expected number of consumers to be greater than 0, but the it is 0."
            )
        elif len(self._consumer) == 1:
            return self.get(timeout=self._timeout)

        with self._frontlock:
            consumer_idx = self._consumer[op_name]
            base_idx = self._consumer_base_idx
            data_idx = consumer_idx - base_idx

            if data_idx >= len(self._frontbatch):
                batch_data = self.get(timeout=self._timeout)
                self._frontbatch.append(batch_data)

            resp = self._frontbatch[data_idx]

            self._idx_consumer_num[consumer_idx] -= 1
            if consumer_idx == base_idx and self._idx_consumer_num[
                    consumer_idx] == 0:
                self._idx_consumer_num.pop(consumer_idx)
                self._frontbatch.pop(0)
                self._consumer_base_idx += 1

            self._consumer[op_name] += 1
            new_consumer_idx = self._consumer[op_name]
            if self._idx_consumer_num.get(new_consumer_idx) is None:
                self._idx_consumer_num[new_consumer_idx] = 0
            self._idx_consumer_num[new_consumer_idx] += 1

        return resp  # reference, read only


class Op(object):
    def __init__(self,
                 name,
                 inputs,
                 in_dtype,
                 outputs,
                 out_dtype,
                 batchsize=1,
                 server_model=None,
                 server_port=None,
                 device=None,
                 client_config=None,
                 server_name=None,
                 fetch_names=None,
                 concurrency=1):
        self._run = False
        # TODO: globally unique check
        self._name = name  # to identify the type of OP, it must be globally unique
        self._concurrency = concurrency  # amount of concurrency
        self.set_inputs(inputs)
        self._in_dtype = in_dtype
        self.set_outputs(outputs)
        self._out_dtype = out_dtype
        self._batch_size = batchsize
        self._client = None
        if client_config is not None and \
                server_name is not None and \
                fetch_names is not None:
            self.set_client(client_config, server_name, fetch_names)
        self._server_model = server_model
        self._server_port = server_port
        self._device = device
        self._data_ids = []

    def set_client(self, client_config, server_name, fetch_names):
        self._client = Client()
        self._client.load_client_config(client_config)
        self._client.connect([server_name])
        self._fetch_names = fetch_names

    def with_serving(self):
        return self._client is not None

    def get_inputs(self):
        return self._inputs

    def set_inputs(self, channels):
        if not isinstance(channels, list):
            raise TypeError('channels must be list type')
        for channel in channels:
            channel.add_consumer(self._name)
        self._inputs = channels

    def get_outputs(self):
        return self._outputs

    def set_outputs(self, channels):
        if not isinstance(channels, list):
            raise TypeError('channels must be list type')
        self._outputs = channels

    def get_data_ids(self):
        return self._data_ids

    def clear_data_ids(self):
        self._data_ids = []

    def append_id_to_data_ids(self, data_id):
        self._data_ids.append(data_id)

    def preprocess(self, input_data):
        if len(input_data) != 1:
            raise Exception(
                'this Op has multiple previous channels. Please override this method'
            )
        feed_batch = []
        self.clear_data_ids()
        for data in input_data:
            if len(data.insts) != self._batch_size:
                raise Exception('len(data_insts) != self._batch_size')
            feed = {}
            for inst in data.insts:
                feed[inst.name] = np.frombuffer(inst.data, dtype=self._in_dtype)
            feed_batch.append(feed)
            self.append_id_to_data_ids(data.id)
        return feed_batch

    def midprocess(self, data):
        # data = preprocess(input), which must be a dict
        logging.debug('data: {}'.format(data))
        logging.debug('fetch: {}'.format(self._fetch_names))
        fetch_map = self._client.predict(feed=data, fetch=self._fetch_names)
        return fetch_map

    def postprocess(self, output_data):
        return output_data

    def stop(self):
        self._run = False

    def start(self):
        self._run = True
        while self._run:
            input_data = []
            for channel in self._inputs:
                input_data.append(channel.front(self._name))
            if len(input_data) > 1:
                data = self.preprocess(input_data)
            else:
                data = self.preprocess(input_data[0])

            if self.with_serving():
                fetch_map = self.midprocess(data)
                output_data = self.postprocess(fetch_map)
            else:
                output_data = self.postprocess(data)

            for channel in self._outputs:
                channel.push(output_data)

    def get_concurrency(self):
        return self._concurrency


class GeneralPythonService(
        general_python_service_pb2_grpc.GeneralPythonService):
    def __init__(self, in_channel, out_channel):
        super(GeneralPythonService, self).__init__()
        self.set_in_channel(in_channel)
        self.set_out_channel(out_channel)
        #TODO: 
        #  multi-lock for different clients
        #  diffenert lock for server and client
        self._id_lock = threading.Lock()
        self._cv = threading.Condition()
        self._globel_resp_dict = {}
        self._id_counter = 0
        self._recive_func = threading.Thread(
            target=GeneralPythonService._recive_out_channel_func, args=(self, ))
        self._recive_func.start()
        logging.debug('succ init')

    def set_in_channel(self, in_channel):
        self._in_channel = in_channel

    def set_out_channel(self, out_channel):
        if isinstance(out_channel, list):
            raise TypeError('out_channel can not be list type')
        out_channel.add_consumer("__GeneralPythonService__")
        self._out_channel = out_channel

    def _recive_out_channel_func(self):
        while True:
            data = self._out_channel.front()
            data_id = None
            for d in data:
                if data_id is None:
                    data_id = d.id
                if data_id != d.id:
                    raise Exception("id not match: {} vs {}".format(data_id,
                                                                    d.id))
            self._cv.acquire()
            self._globel_resp_dict[data_id] = data
            self._cv.notify_all()
            self._cv.release()

    def _get_next_id(self):
        with self._id_lock:
            self._id_counter += 1
            return self._id_counter - 1

    def _get_data_in_globel_resp_dict(self, data_id):
        self._cv.acquire()
        while data_id not in self._globel_resp_dict:
            self._cv.wait()
        resp = self._globel_resp_dict.pop(data_id)
        self._cv.notify_all()
        self._cv.release()
        return resp

    def _pack_data_for_infer(self, request):
        logging.debug('start inferce')
        data = python_service_channel_pb2.ChannelData()
        data_id = self._get_next_id()
        data.id = data_id
        for idx, name in enumerate(request.feed_var_names):
            logging.debug('name: {}'.format(request.feed_var_names[idx]))
            logging.debug('data: {}'.format(request.feed_insts[idx]))
            inst = python_service_channel_pb2.Inst()
            inst.data = request.feed_insts[idx]
            inst.name = name
            data.insts.append(inst)
        return data, data_id

    def _pack_data_for_resp(self, data):
        data = data[0]  #TODO batchsize = 1
        logging.debug('get data')
        resp = general_python_service_pb2.Response()
        logging.debug('gen resp')
        logging.debug(data)
        for inst in data.insts:
            logging.debug('append data')
            resp.fetch_insts.append(inst.data)
            logging.debug('append name')
            resp.fetch_var_names.append(inst.name)
        return resp

    def inference(self, request, context):
        data, data_id = self._pack_data_for_infer(request)
        logging.debug('push data')
        self._in_channel.push(data)
        logging.debug('wait for infer')
        resp_data = None
        resp_data = self._get_data_in_globel_resp_dict(data_id)
        resp = self._pack_data_for_resp(resp_data)
        return resp


class PyServer(object):
    def __init__(self):
        self._channels = []
        self._ops = []
        self._op_threads = []
        self._port = None
        self._worker_num = None
        self._in_channel = None
        self._out_channel = None

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
            inputs |= set(op.get_inputs())
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

    def _op_start_wrapper(self, op):
        return op.start()

    def _run_ops(self):
        for op in self._ops:
            op_concurrency = op.get_concurrency()
            for c in range(op_concurrency):
                # th = multiprocessing.Process(target=self._op_start_wrapper, args=(op, ))
                th = threading.Thread(
                    target=self._op_start_wrapper, args=(op, ))
                th.start()
                self._op_threads.append(th)

    def run_server(self):
        self._run_ops()
        server = grpc.server(
            futures.ThreadPoolExecutor(max_workers=self._worker_num))
        general_python_service_pb2_grpc.add_GeneralPythonServiceServicer_to_server(
            GeneralPythonService(self._in_channel, self._out_channel), server)
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

        # run a server (not in PyServing)
        if device == "cpu":
            cmd = "python -m paddle_serving_server.serve --model {} --thread 4 --port {} &>/dev/null &".format(
                model_path, port)
        else:
            cmd = "python -m paddle_serving_server_gpu.serve --model {} --thread 4 --port {} &>/dev/null &".format(
                model_path, port)
        logging.info(cmd)
        return
        # os.system(cmd)
