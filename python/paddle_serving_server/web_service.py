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
import numpy as np
import os
from paddle_serving_server import pipeline
from paddle_serving_server.pipeline import Op


def port_is_available(port):
    with closing(socket.socket(socket.AF_INET, socket.SOCK_STREAM)) as sock:
        sock.settimeout(2)
        result = sock.connect_ex(('0.0.0.0', port))
    if result != 0:
        return True
    else:
        return False


class WebService(object):
    def __init__(self, name="default_service"):
        self.name = name
        # pipeline
        self._server = pipeline.PipelineServer(self.name)

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

    def load_model_config(self, server_config_dir_paths, client_config_path=None):
        if isinstance(server_config_dir_paths, str):
            server_config_dir_paths = [server_config_dir_paths]
        elif isinstance(server_config_dir_paths, list):
            pass

        for single_model_config in server_config_dir_paths:
            if os.path.isdir(single_model_config):
                pass
            elif os.path.isfile(single_model_config):
                raise ValueError("The input of --model should be a dir not file.")
        self.server_config_dir_paths = server_config_dir_paths
        from .proto import general_model_config_pb2 as m_config
        import google.protobuf.text_format
        file_path_list = []
        for single_model_config in self.server_config_dir_paths:
            file_path_list.append( "{}/serving_server_conf.prototxt".format(single_model_config) )
        
        model_conf = m_config.GeneralModelConfig()
        f = open(file_path_list[0], 'r')
        model_conf = google.protobuf.text_format.Merge(
            str(f.read()), model_conf)
        self.feed_vars = {var.name: var for var in model_conf.feed_var}

        if len(file_path_list) > 1:
            model_conf = m_config.GeneralModelConfig()
            f = open(file_path_list[-1], 'r')
            model_conf = google.protobuf.text_format.Merge(
                str(f.read()), model_conf)

        self.fetch_vars = {var.name: var for var in model_conf.fetch_var}
        if client_config_path == None:
            self.client_config_path = self.server_config_dir_paths

    def _launch_rpc_service(self):
        op_maker = OpMaker()
        op_seq_maker = OpSeqMaker()

        read_op = op_maker.create('general_reader')
        op_seq_maker.add_op(read_op)

        for idx, single_model in enumerate(self.server_config_dir_paths):
            infer_op_name = "general_infer"
            if len(self.server_config_dir_paths) == 2 and idx == 0:
                infer_op_name = "general_detection"
            else:
                infer_op_name = "general_infer"
            general_infer_op = op_maker.create(infer_op_name)
            op_seq_maker.add_op(general_infer_op)
        
        general_response_op = op_maker.create('general_response')
        op_seq_maker.add_op(general_response_op)

        server = Server()
        server.set_op_sequence(op_seq_maker.get_op_sequence())
        server.set_num_threads(16)
        server.set_memory_optimize(self.mem_optim)
        server.set_ir_optimize(self.ir_optim)
        server.load_model_config(self.server_config_dir_paths)#brpc Server support server_config_dir_paths 
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
        print("This API will be deprecated later. Please do not use it")
        self.workdir = workdir
        self.port = port
        self.device = device
        default_port = 12000
        self.port_list = []
        self.mem_optim = mem_optim
        self.ir_optim = ir_optim
        for i in range(1000):
            if port_is_available(default_port + i):
                self.port_list.append(default_port + i)
                break

    def _launch_web_service(self):
        self.client = Client()
        self.client.load_client_config(self.client_config_path)
        self.client.connect(["0.0.0.0:{}".format(self.port_list[0])])

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

    def run_debugger_service(self):
        import socket
        localIP = socket.gethostbyname(socket.gethostname())
        print("web service address:")
        print("http://{}:{}/{}/prediction".format(localIP, self.port,
                                                  self.name))
        app_instance = Flask(__name__)

        @app_instance.before_first_request
        def init():
            self._launch_local_predictor()

        service_name = "/" + self.name + "/prediction"

        @app_instance.route(service_name, methods=["POST"])
        def run():
            return self.get_prediction(request)

        self.app_instance = app_instance

    def _launch_local_predictor(self):
        from paddle_serving_app.local_predict import LocalPredictor
        self.client = LocalPredictor()
        # actually, LocalPredictor is like a server, but it is WebService Request initiator
        # for WebService it is a Client.
        # local_predictor only support single-Model DirPath - Type:str
        # so the input must be self.server_config_dir_paths[0]
        self.client.load_model_config(self.server_config_dir_paths[0], use_gpu=False)

    def run_web_service(self):
        print("This API will be deprecated later. Please do not use it")
        self.app_instance.run(host="0.0.0.0", port=self.port, threaded=True)

    def get_app_instance(self):
        return self.app_instance

    def preprocess(self, feed=[], fetch=[]):
        print("This API will be deprecated later. Please do not use it")
        is_batch = True
        feed_dict = {}
        for var_name in self.feed_vars.keys():
            feed_dict[var_name] = []
        for feed_ins in feed:
            for key in feed_ins:
                feed_dict[key].append(
                    np.array(feed_ins[key]).reshape(
                        list(self.feed_vars[key].shape))[np.newaxis, :])
        feed = {}
        for key in feed_dict:
            feed[key] = np.concatenate(feed_dict[key], axis=0)
        return feed, fetch, is_batch

    def postprocess(self, feed=[], fetch=[], fetch_map=None):
        print("This API will be deprecated later. Please do not use it")
        for key in fetch_map:
            fetch_map[key] = fetch_map[key].tolist()
        return fetch_map
