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

from flask import Flask, request, abort
from contextlib import closing
from multiprocessing import Pool, Process, Queue
from paddle_serving_client import Client
from paddle_serving_server_gpu import OpMaker, OpSeqMaker, Server
from paddle_serving_server_gpu.serve import start_multi_card
import socket
import sys
import numpy as np
import paddle_serving_server_gpu as serving

from paddle_serving_server_gpu import pipeline
from paddle_serving_server_gpu.pipeline import Op


class WebService(object):
    def __init__(self, name="default_service"):
        self.name = name
        # pipeline
        self._server = pipeline.PipelineServer(self.name)

        self.gpus = []  # deprecated
        self.rpc_service_list = []  # deprecated

    def get_pipeline_response(self, read_op):
        return None

    def prepare_pipeline_config(self, yaml_file):
        # build dag
        read_op = pipeline.RequestOp()
        last_op = self.get_pipeline_response(read_op)
        if not isinstance(last_op, Op):
            raise ValueError("The return value type of `get_pipeline_response` "
                             "function is not Op type, please check function "
                             "`get_pipeline_response`.")
        response_op = pipeline.ResponseOp(input_ops=[last_op])
        self._server.set_response_op(response_op)
        self._server.prepare_server(yaml_file)

    def run_service(self):
        self._server.run_server()

    def load_model_config(self, model_config):
        print("This API will be deprecated later. Please do not use it")
        self.model_config = model_config
        import os
        from .proto import general_model_config_pb2 as m_config
        import google.protobuf.text_format
        if os.path.isdir(model_config):
            client_config = "{}/serving_server_conf.prototxt".format(
                model_config)
        elif os.path.isfile(model_config):
            client_config = model_config
        model_conf = m_config.GeneralModelConfig()
        f = open(client_config, 'r')
        model_conf = google.protobuf.text_format.Merge(
            str(f.read()), model_conf)
        self.feed_names = [var.alias_name for var in model_conf.feed_var]
        self.fetch_names = [var.alias_name for var in model_conf.fetch_var]

    def set_gpus(self, gpus):
        print("This API will be deprecated later. Please do not use it")
        self.gpus = [int(x) for x in gpus.split(",")]

    def default_rpc_service(self,
                            workdir="conf",
                            port=9292,
                            gpuid=0,
                            thread_num=2,
                            mem_optim=True,
                            ir_optim=False):
        device = "gpu"
        if gpuid == -1:
            device = "cpu"
        op_maker = serving.OpMaker()
        read_op = op_maker.create('general_reader')
        general_infer_op = op_maker.create('general_infer')
        general_response_op = op_maker.create('general_response')

        op_seq_maker = OpSeqMaker()
        op_seq_maker.add_op(read_op)
        op_seq_maker.add_op(general_infer_op)
        op_seq_maker.add_op(general_response_op)

        server = Server()
        server.set_op_sequence(op_seq_maker.get_op_sequence())
        server.set_num_threads(thread_num)
        server.set_memory_optimize(mem_optim)
        server.set_ir_optimize(ir_optim)

        server.load_model_config(self.model_config)
        if gpuid >= 0:
            server.set_gpuid(gpuid)
        server.prepare_server(workdir=workdir, port=port, device=device)
        return server

    def _launch_rpc_service(self, service_idx):
        self.rpc_service_list[service_idx].run_server()

    def port_is_available(self, port):
        with closing(socket.socket(socket.AF_INET, socket.SOCK_STREAM)) as sock:
            sock.settimeout(2)
            result = sock.connect_ex(('0.0.0.0', port))
        if result != 0:
            return True
        else:
            return False

    def prepare_server(self,
                       workdir="",
                       port=9393,
                       device="gpu",
                       gpuid=0,
                       mem_optim=True,
                       ir_optim=False):
        print("This API will be deprecated later. Please do not use it")
        self.workdir = workdir
        self.port = port
        self.device = device
        self.gpuid = gpuid
        self.port_list = []
        default_port = 12000
        for i in range(1000):
            if self.port_is_available(default_port + i):
                self.port_list.append(default_port + i)
            if len(self.port_list) > len(self.gpus):
                break

        if len(self.gpus) == 0:
            # init cpu service
            self.rpc_service_list.append(
                self.default_rpc_service(
                    self.workdir,
                    self.port_list[0],
                    -1,
                    thread_num=2,
                    mem_optim=mem_optim,
                    ir_optim=ir_optim))
        else:
            for i, gpuid in enumerate(self.gpus):
                self.rpc_service_list.append(
                    self.default_rpc_service(
                        "{}_{}".format(self.workdir, i),
                        self.port_list[i],
                        gpuid,
                        thread_num=2,
                        mem_optim=mem_optim,
                        ir_optim=ir_optim))

    def _launch_web_service(self):
        gpu_num = len(self.gpus)
        self.client = Client()
        self.client.load_client_config("{}/serving_server_conf.prototxt".format(
            self.model_config))
        endpoints = ""
        if gpu_num > 0:
            for i in range(gpu_num):
                endpoints += "127.0.0.1:{},".format(self.port_list[i])
        else:
            endpoints = "127.0.0.1:{}".format(self.port_list[0])
        self.client.connect([endpoints])

    def get_prediction(self, request):
        if not request.json:
            abort(400)
        if "fetch" not in request.json:
            abort(400)
        try:
            feed, fetch, is_batch = self.preprocess(request.json["feed"],
                                                    request.json["fetch"])
            if isinstance(feed, dict) and "fetch" in feed:
                del feed["fetch"]
            if len(feed) == 0:
                raise ValueError("empty input")
            fetch_map = self.client.predict(
                feed=feed, fetch=fetch, batch=is_batch)
            result = self.postprocess(
                feed=request.json["feed"], fetch=fetch, fetch_map=fetch_map)
            result = {"result": result}
        except ValueError as err:
            result = {"result": str(err)}
        return result

    def run_rpc_service(self):
        print("This API will be deprecated later. Please do not use it")
        import socket
        localIP = socket.gethostbyname(socket.gethostname())
        print("web service address:")
        print("http://{}:{}/{}/prediction".format(localIP, self.port,
                                                  self.name))
        server_pros = []
        for i, service in enumerate(self.rpc_service_list):
            p = Process(target=self._launch_rpc_service, args=(i, ))
            server_pros.append(p)
        for p in server_pros:
            p.start()

        app_instance = Flask(__name__)

        @app_instance.before_first_request
        def init():
            self._launch_web_service()

        service_name = "/" + self.name + "/prediction"

        @app_instance.route(service_name, methods=["POST"])
        def run():
            return self.get_prediction(request)

        self.app_instance = app_instance

    # TODO: maybe change another API name: maybe run_local_predictor?
    def run_debugger_service(self, gpu=False):
        print("This API will be deprecated later. Please do not use it")
        import socket
        localIP = socket.gethostbyname(socket.gethostname())
        print("web service address:")
        print("http://{}:{}/{}/prediction".format(localIP, self.port,
                                                  self.name))
        app_instance = Flask(__name__)

        @app_instance.before_first_request
        def init():
            self._launch_local_predictor(gpu)

        service_name = "/" + self.name + "/prediction"

        @app_instance.route(service_name, methods=["POST"])
        def run():
            return self.get_prediction(request)

        self.app_instance = app_instance

    def _launch_local_predictor(self, gpu):
        from paddle_serving_app.local_predict import LocalPredictor
        self.client = LocalPredictor()
        self.client.load_model_config(
            "{}".format(self.model_config), use_gpu=True, gpu_id=self.gpus[0])

    def run_web_service(self):
        print("This API will be deprecated later. Please do not use it")
        self.app_instance.run(host="0.0.0.0", port=self.port, threaded=True)

    def get_app_instance(self):
        return self.app_instance

    def preprocess(self, feed=[], fetch=[]):
        print("This API will be deprecated later. Please do not use it")
        is_batch = True
        return feed, fetch, is_batch

    def postprocess(self, feed=[], fetch=[], fetch_map=None):
        print("This API will be deprecated later. Please do not use it")
        for key in fetch_map:
            fetch_map[key] = fetch_map[key].tolist()
        return fetch_map
