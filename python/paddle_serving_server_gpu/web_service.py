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
#!flask/bin/python
# pylint: disable=doc-string-missing

from flask import Flask, request, abort
from multiprocessing import Pool, Process
from paddle_serving_server_gpu import OpMaker, OpSeqMaker, Server
import paddle_serving_server_gpu as serving
from paddle_serving_client import Client
from .serve import start_multi_card
import time
import random


class WebService(object):
    def __init__(self, name="default_service"):
        self.name = name
        self.gpus = []
        self.rpc_service_list = []

    def load_model_config(self, model_config):
        self.model_config = model_config

    def set_gpus(self, gpus):
        self.gpus = gpus

    def default_rpc_service(self,
                            workdir="conf",
                            port=9292,
                            gpuid=0,
                            thread_num=10):
        device = "gpu"
        if gpuid == -1:
            device = "cpu"
        op_maker = serving.OpMaker()
        read_op = op_maker.create('general_reader')
        general_infer_op = op_maker.create('general_infer')
        general_response_op = op_maker.create('general_response')

        op_seq_maker = serving.OpSeqMaker()
        op_seq_maker.add_op(read_op)
        op_seq_maker.add_op(general_infer_op)
        op_seq_maker.add_op(general_response_op)

        server = serving.Server()
        server.set_op_sequence(op_seq_maker.get_op_sequence())
        server.set_num_threads(thread_num)

        server.load_model_config(self.model_config)
        if gpuid >= 0:
            server.set_gpuid(gpuid)
        server.prepare_server(workdir=workdir, port=port, device=device)
        return server

    def _launch_rpc_service(self, service_idx):
        self.rpc_service_list[service_idx].run_server()

    def prepare_server(self, workdir="", port=9393, device="gpu", gpuid=0):
        self.workdir = workdir
        self.port = port
        self.device = device
        self.gpuid = gpuid
        if len(self.gpus) == 0:
            # init cpu service
            self.rpc_service_list.append(
                self.default_rpc_service(
                    self.workdir, self.port + 1, -1, thread_num=10))
        else:
            for i, gpuid in enumerate(self.gpus):
                self.rpc_service_list.append(
                    self.default_rpc_service(
                        "{}_{}".format(self.workdir, i),
                        self.port + 1 + i,
                        gpuid,
                        thread_num=10))

    def _launch_web_service(self, gpu_num):
        app_instance = Flask(__name__)
        client_list = []
        if gpu_num > 1:
            gpu_num = 0
        for i in range(gpu_num):
            client_service = Client()
            client_service.load_client_config(
                "{}/serving_server_conf.prototxt".format(self.model_config))
            client_service.connect(["127.0.0.1:{}".format(self.port + i + 1)])
            client_list.append(client_service)
            time.sleep(1)
        service_name = "/" + self.name + "/prediction"

        @app_instance.route(service_name, methods=['POST'])
        def get_prediction():
            if not request.json:
                abort(400)
            if "fetch" not in request.json:
                abort(400)
            feed, fetch = self.preprocess(request.json, request.json["fetch"])
            if "fetch" in feed:
                del feed["fetch"]
            fetch_map = client_list[0].predict(feed=feed, fetch=fetch)
            fetch_map = self.postprocess(
                feed=request.json, fetch=fetch, fetch_map=fetch_map)
            return fetch_map

        app_instance.run(host="0.0.0.0",
                         port=self.port,
                         threaded=False,
                         processes=1)

    def run_server(self):
        import socket
        localIP = socket.gethostbyname(socket.gethostname())
        print("web service address:")
        print("http://{}:{}/{}/prediction".format(localIP, self.port,
                                                  self.name))

        rpc_processes = []
        for idx in range(len(self.rpc_service_list)):
            p_rpc = Process(target=self._launch_rpc_service, args=(idx, ))
            rpc_processes.append(p_rpc)

        for p in rpc_processes:
            p.start()

        p_web = Process(
            target=self._launch_web_service, args=(len(self.gpus), ))
        p_web.start()
        for p in rpc_processes:
            p.join()
        p_web.join()

    def preprocess(self, feed={}, fetch=[]):
        return feed, fetch

    def postprocess(self, feed={}, fetch=[], fetch_map={}):
        return fetch_map
