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
import numpy
import grpc
import general_python_service_pb2
import general_python_service_pb2_grpc
import python_service_channel_pb2


class Channel(Queue.Queue):
    def __init__(self, consumer=1, maxsize=-1, timeout=None, batchsize=1):
        Queue.Queue.__init__(self, maxsize=maxsize)
        # super(Channel, self).__init__(maxsize=maxsize)
        self._maxsize = maxsize
        self._timeout = timeout
        self._batchsize = batchsize
        self._consumer = consumer
        self._pushlock = threading.Lock()
        self._frontlock = threading.Lock()
        self._pushbatch = []
        self._frontbatch = None
        self._count = 0

    def push(self, item):
        with self._pushlock:
            self._pushbatch.append(item)
            if len(self._pushbatch) == self._batchsize:
                self.put(self._pushbatch, timeout=self._timeout)
                # self.put(self._pushbatch)
                self._pushbatch = []

    def front(self):
        if self._consumer == 1:
            return self.get(timeout=self._timeout)
        with self._frontlock:
            if self._count == 0:
                self._frontbatch = self.get(timeout=self._timeout)
            self._count += 1
            if self._count == self._consumer:
                self._count = 0
            return self._frontbatch


class Op(object):
    def __init__(self,
                 inputs,
                 outputs,
                 server_model=None,
                 server_port=None,
                 device=None,
                 client_config=None,
                 server_name=None,
                 fetch_names=None):
        self._run = False
        self.set_inputs(inputs)
        self.set_outputs(outputs)
        self._client = None
        if client_config is not None and \
                server_name is not None and \
                fetch_names is not None:
            self.set_client(client_config, server_name, fetch_names)
        self._server_model = server_model
        self._server_port = server_port
        self._device = device

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
        self._inputs = channels

    def get_outputs(self):
        return self._outputs

    def set_outputs(self, channels):
        if not isinstance(channels, list):
            raise TypeError('channels must be list type')
        self._outputs = channels

    def preprocess(self, input_data):
        return input_data

    def midprocess(self, data):
        # data = preprocess(input), which must be a dict
        print('data: {}'.format(data))
        print('fetch: {}'.format(self._fetch_names))
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
                input_data.append(channel.front())
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


class GeneralPythonService(
        general_python_service_pb2_grpc.GeneralPythonService):
    def __init__(self, in_channel, out_channel):
        super(GeneralPythonService, self).__init__()
        self._in_channel = in_channel
        self._out_channel = out_channel
        print('succ init')

    def inference(self, request, context):
        print('start inferce')
        data = python_service_channel_pb2.ChannelData()
        print('gen data: {}'.format(data))
        for idx, name in enumerate(request.feed_var_names):
            print('name: {}'.format(request.feed_var_names[idx]))
            print('data: {}'.format(request.feed_insts[idx]))
            inst = python_service_channel_pb2.Inst()
            inst.data = request.feed_insts[idx]
            inst.name = name
            inst.id = 0  #TODO
            data.insts.append(inst)
        print('push data')
        self._in_channel.push(data)
        print('wait for infer')
        data = self._out_channel.front()
        data = data[0]  #TODO batchsize = 1
        print('get data')
        resp = general_python_service_pb2.Response()
        print('gen resp')
        print(data)
        for inst in data.insts:
            print('append data')
            resp.fetch_insts.append(inst.data)
            print('append name')
            resp.fetch_var_names.append(inst.name)
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
        print('here will generate desc for paas')
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

    def op_start_wrapper(self, op):
        return op.start()

    def run_server(self):
        for op in self._ops:
            # th = multiprocessing.Process(target=self.op_start_wrapper, args=(op, ))
            th = threading.Thread(target=self.op_start_wrapper, args=(op, ))
            th.start()
            self._op_threads.append(th)
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
        print(cmd)
        return
        os.system(cmd)
