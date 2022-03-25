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

# Now, this is only for Pipeline.
from flask import Flask, request, abort
from contextlib import closing
from multiprocessing import Pool, Process, Queue
from paddle_serving_client import Client
from paddle_serving_server import OpMaker, OpSeqMaker, Server
from paddle_serving_server.serve import start_multi_card
import socket
import sys
import numpy as np
import os
from paddle_serving_server import pipeline
from paddle_serving_server.pipeline import Op
from paddle_serving_server.serve import format_gpu_to_strlist
from paddle_serving_server.util import dump_pid_file


def port_is_available(port):
    with closing(socket.socket(socket.AF_INET, socket.SOCK_STREAM)) as sock:
        sock.settimeout(2)
        result = sock.connect_ex(('127.0.0.1', port))
    if result != 0:
        return True
    else:
        return False


class WebService(object):
    def __init__(self, name="default_service"):
        self.name = name
        # pipeline
        self._server = pipeline.PipelineServer(self.name)

        self.gpus = ["-1"]  # deprecated
        self.rpc_service_list = []  # deprecated

    def get_pipeline_response(self, read_op):
        return None

    def prepare_pipeline_config(self, yml_file=None, yml_dict=None):
        # build dag
        read_op = pipeline.RequestOp()
        last_op = self.get_pipeline_response(read_op)
        if not isinstance(last_op, Op):
            raise ValueError("The return value type of `get_pipeline_response` "
                             "function is not Op type, please check function "
                             "`get_pipeline_response`.")
        response_op = pipeline.ResponseOp(input_ops=[last_op])
        self._server.set_response_op(response_op)
        self._server.prepare_server(yml_file=yml_file, yml_dict=yml_dict)

    def run_service(self):
        self._server.run_server()

    def load_model_config(self,
                          server_config_dir_paths,
                          client_config_path=None):
        if isinstance(server_config_dir_paths, str):
            server_config_dir_paths = [server_config_dir_paths]
        elif isinstance(server_config_dir_paths, list):
            pass

        for single_model_config in server_config_dir_paths:
            if os.path.isdir(single_model_config):
                pass
            elif os.path.isfile(single_model_config):
                raise ValueError(
                    "The input of --model should be a dir not file.")
        self.server_config_dir_paths = server_config_dir_paths
        from .proto import general_model_config_pb2 as m_config
        import google.protobuf.text_format
        file_path_list = []
        for single_model_config in self.server_config_dir_paths:
            file_path_list.append("{}/serving_server_conf.prototxt".format(
                single_model_config))

        model_conf = m_config.GeneralModelConfig()
        f = open(file_path_list[0], 'r')
        model_conf = google.protobuf.text_format.Merge(
            str(f.read()), model_conf)
        self.feed_vars = {var.alias_name: var for var in model_conf.feed_var}

        if len(file_path_list) > 1:
            model_conf = m_config.GeneralModelConfig()
            f = open(file_path_list[-1], 'r')
            model_conf = google.protobuf.text_format.Merge(
                str(f.read()), model_conf)

        self.fetch_vars = {var.alias_name: var for var in model_conf.fetch_var}
        if client_config_path == None:
            self.client_config_path = file_path_list

    # after this function, self.gpus should be a list of str or [].
    def set_gpus(self, gpus):
        print("This API will be deprecated later. Please do not use it")
        self.gpus = format_gpu_to_strlist(gpus)

# this function can be called by user
# or by Function create_rpc_config
# if by user, user can set_gpus or pass the `gpus`
# if `gpus` == None, which means it`s not set at all.
# at this time, we should use self.gpus instead.
# otherwise, we should use the `gpus` first.
# which means if set_gpus and `gpus` is both set.
# `gpus` will be used.

    def default_rpc_service(self,
                            workdir,
                            port=9292,
                            gpus=None,
                            thread_num=4,
                            mem_optim=True,
                            use_lite=False,
                            use_xpu=False,
                            ir_optim=False,
                            precision="fp32",
                            use_calib=False,
                            use_trt=False,
                            gpu_multi_stream=False,
                            runtime_thread_num=None,
                            batch_infer_size=None):

        device = "cpu"
        server = Server()
        # only when `gpus == None`, which means it`s not set at all
        # we will use the self.gpus.
        if gpus == None:
            gpus = self.gpus

        gpus = format_gpu_to_strlist(gpus)
        server.set_gpuid(gpus)

        if len(gpus) == 0 or gpus == ["-1"]:
            if use_lite:
                device = "arm"
            else:
                device = "cpu"
        else:
            device = "gpu"

        op_maker = OpMaker()
        op_seq_maker = OpSeqMaker()

        read_op = op_maker.create('GeneralReaderOp')
        op_seq_maker.add_op(read_op)

        for idx, single_model in enumerate(self.server_config_dir_paths):
            infer_op_name = "GeneralInferOp"
            if len(self.server_config_dir_paths) == 2 and idx == 0:
                infer_op_name = "GeneralDetectionOp"
            else:
                infer_op_name = "GeneralInferOp"
            general_infer_op = op_maker.create(infer_op_name)
            op_seq_maker.add_op(general_infer_op)

        general_response_op = op_maker.create('GeneralResponseOp')
        op_seq_maker.add_op(general_response_op)

        server.set_op_sequence(op_seq_maker.get_op_sequence())
        server.set_num_threads(thread_num)
        server.set_memory_optimize(mem_optim)
        server.set_ir_optimize(ir_optim)
        server.set_device(device)
        server.set_precision(precision)
        server.set_use_calib(use_calib)

        if use_trt and device == "gpu":
            server.set_trt()
            server.set_ir_optimize(True)

        if gpu_multi_stream and device == "gpu":
            server.set_gpu_multi_stream()

        if runtime_thread_num:
            server.set_runtime_thread_num(runtime_thread_num)

        if batch_infer_size:
            server.set_batch_infer_size(batch_infer_size)

        if use_lite:
            server.set_lite()
        if use_xpu:
            server.set_xpu()

        server.load_model_config(self.server_config_dir_paths
                                 )  #brpc Server support server_config_dir_paths

        server.prepare_server(workdir=workdir, port=port, device=device)
        return server

    def _launch_rpc_service(self, service_idx):
        self.rpc_service_list[service_idx].run_server()

    # if use this function, self.gpus must be set before.
    # if not, we will use the default value, self.gpus = ["-1"].
    # so we always pass the `gpus` = self.gpus. 
    def create_rpc_config(self):
        self.rpc_service_list.append(
            self.default_rpc_service(
                self.workdir,
                self.port_list[0],
                self.gpus,
                thread_num=self.thread_num,
                mem_optim=self.mem_optim,
                use_lite=self.use_lite,
                use_xpu=self.use_xpu,
                ir_optim=self.ir_optim,
                precision=self.precision,
                use_calib=self.use_calib,
                use_trt=self.use_trt,
                gpu_multi_stream=self.gpu_multi_stream,
                runtime_thread_num=self.runtime_thread_num,
                batch_infer_size=self.batch_infer_size))

    def prepare_server(self,
                       workdir,
                       port=9393,
                       device="cpu",
                       precision="fp32",
                       use_calib=False,
                       use_lite=False,
                       use_xpu=False,
                       ir_optim=False,
                       thread_num=4,
                       mem_optim=True,
                       use_trt=False,
                       gpu_multi_stream=False,
                       runtime_thread_num=None,
                       batch_infer_size=None,
                       gpuid=None):
        print("This API will be deprecated later. Please do not use it")
        self.workdir = workdir
        self.port = port
        self.thread_num = thread_num
        # self.device is not used at all.
        # device is set by gpuid.
        self.precision = precision
        self.use_calib = use_calib
        self.use_lite = use_lite
        self.use_xpu = use_xpu
        self.ir_optim = ir_optim
        self.mem_optim = mem_optim
        self.port_list = []
        self.use_trt = use_trt
        self.gpu_multi_stream = gpu_multi_stream
        self.runtime_thread_num = runtime_thread_num
        self.batch_infer_size = batch_infer_size
            
        # record port and pid info for stopping process
        dump_pid_file([self.port], "web_service")
        # if gpuid != None, we will use gpuid first.
        # otherwise, keep the self.gpus unchanged.
        # maybe self.gpus is set by the Function set_gpus.
        if gpuid != None:
            self.gpus = format_gpu_to_strlist(gpuid)
        else:
            pass

        default_port = 12000
        for i in range(1000):
            if port_is_available(default_port + i):
                self.port_list.append(default_port + i)
                break

    def _launch_web_service(self):
        self.client = Client()
        self.client.load_client_config(self.client_config_path)
        endpoints = ""
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
        self.create_rpc_config()
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
        # actually, LocalPredictor is like a server, but it is WebService Request initiator
        # for WebService it is a Client.
        # local_predictor only support single-Model DirPath - Type:str
        # so the input must be self.server_config_dir_paths[0]
        from paddle_serving_app.local_predict import LocalPredictor
        self.client = LocalPredictor()
        if gpu:
            # if user forget to call function `set_gpus` to set self.gpus.
            # default self.gpus = [0].
            if len(self.gpus) == 0 or self.gpus == ["-1"]:
                self.gpus = ["0"]
            # right now, local Predictor only support 1 card.
            # no matter how many gpu_id is in gpus, we only use the first one.
            gpu_id = (self.gpus[0].split(","))[0]
            self.client.load_model_config(
                self.server_config_dir_paths[0], use_gpu=True, gpu_id=gpu_id)
        else:
            self.client.load_model_config(
                self.server_config_dir_paths[0], use_gpu=False)

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
