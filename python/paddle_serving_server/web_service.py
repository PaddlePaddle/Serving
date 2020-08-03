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
from paddle_serving_server import OpMaker, OpSeqMaker, Server
from paddle_serving_client import Client
from contextlib import closing
import socket


class WebService(object):
    def __init__(self, name="default_service"):
        self.name = name

    def load_model_config(self, model_config):
        self.model_config = model_config

    def _launch_rpc_service(self):
        op_maker = OpMaker()
        read_op = op_maker.create('general_reader')
        general_infer_op = op_maker.create('general_infer')
        general_response_op = op_maker.create('general_response')
        op_seq_maker = OpSeqMaker()
        op_seq_maker.add_op(read_op)
        op_seq_maker.add_op(general_infer_op)
        op_seq_maker.add_op(general_response_op)
        server = Server()
        server.set_op_sequence(op_seq_maker.get_op_sequence())
        server.set_num_threads(16)
        server.set_memory_optimize(self.mem_optim)
        server.set_ir_optimize(self.ir_optim)
        server.load_model_config(self.model_config)
        server.prepare_server(
            workdir=self.workdir, port=self.port_list[0], device=self.device)
        server.run_server()

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
                       device="cpu",
                       mem_optim=True,
                       ir_optim=False):
        self.workdir = workdir
        self.port = port
        self.device = device
        default_port = 12000
        self.port_list = []
        self.mem_optim = mem_optim
        self.ir_optim = ir_optim
        for i in range(1000):
            if self.port_is_available(default_port + i):
                self.port_list.append(default_port + i)
                break

    def _launch_web_service(self):
        self.client = Client()
        self.client.load_client_config("{}/serving_server_conf.prototxt".format(
            self.model_config))
        self.client.connect(["0.0.0.0:{}".format(self.port_list[0])])

    def get_prediction(self, request):
        if not request.json:
            abort(400)
        if "fetch" not in request.json:
            abort(400)
        try:
            feed, fetch = self.preprocess(request.json["feed"],
                                          request.json["fetch"])
            if isinstance(feed, dict) and "fetch" in feed:
                del feed["fetch"]
            fetch_map = self.client.predict(feed=feed, fetch=fetch)
            result = self.postprocess(
                feed=request.json["feed"], fetch=fetch, fetch_map=fetch_map)
            result = {"result": result}
        except ValueError as err:
            result = {"result": err}
        return result

    def run_rpc_service(self):
        import socket
        localIP = socket.gethostbyname(socket.gethostname())
        print("web service address:")
        print("http://{}:{}/{}/prediction".format(localIP, self.port,
                                                  self.name))
        p_rpc = Process(target=self._launch_rpc_service)
        p_rpc.start()

        app_instance = Flask(__name__)

        @app_instance.before_first_request
        def init():
            self._launch_web_service()

        service_name = "/" + self.name + "/prediction"

        @app_instance.route(service_name, methods=["POST"])
        def run():
            return self.get_prediction(request)

        self.app_instance = app_instance

    def run_web_service(self):
        self.app_instance.run(host="0.0.0.0",
                              port=self.port,
                              threaded=False,
                              processes=1)

    def get_app_instance(self):
        return self.app_instance

    def preprocess(self, feed=[], fetch=[]):
        return feed, fetch

    def postprocess(self, feed=[], fetch=[], fetch_map=None):
        for key in fetch_map:
            fetch_map[key] = fetch_map[key].tolist()
        return fetch_map
