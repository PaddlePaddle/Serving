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
        server.load_model_config(self.model_config)
        server.prepare_server(
            workdir=self.workdir, port=self.port + 1, device=self.device)
        server.run_server()

    def prepare_server(self, workdir="", port=9393, device="cpu"):
        self.workdir = workdir
        self.port = port
        self.device = device

    def _launch_web_service(self):
        app_instance = Flask(__name__)
        client_service = Client()
        client_service.load_client_config(
            "{}/serving_server_conf.prototxt".format(self.model_config))
        client_service.connect(["0.0.0.0:{}".format(self.port + 1)])
        service_name = "/" + self.name + "/prediction"

        @app_instance.route(service_name, methods=['POST'])
        def get_prediction():
            if not request.json:
                abort(400)
            if "fetch" not in request.json:
                abort(400)
            try:
                feed, fetch = self.preprocess(request.json,
                                              request.json["fetch"])
                if isinstance(feed, list):
                    fetch_map_batch = client_service.predict(
                        feed_batch=feed, fetch=fetch)
                    fetch_map_batch = self.postprocess(
                        feed=request.json,
                        fetch=fetch,
                        fetch_map=fetch_map_batch)
                    result = {"result": fetch_map_batch}
                elif isinstance(feed, dict):
                    if "fetch" in feed:
                        del feed["fetch"]
                    fetch_map = client_service.predict(feed=feed, fetch=fetch)
                    result = self.postprocess(
                        feed=request.json, fetch=fetch, fetch_map=fetch_map)
            except ValueError:
                result = {"result": "Request Value Error"}
            return result

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
        p_rpc = Process(target=self._launch_rpc_service)
        p_web = Process(target=self._launch_web_service)
        p_rpc.start()
        p_web.start()
        p_web.join()
        p_rpc.join()

    def preprocess(self, feed={}, fetch=[]):
        return feed, fetch

    def postprocess(self, feed={}, fetch=[], fetch_map=None):
        return fetch_map
