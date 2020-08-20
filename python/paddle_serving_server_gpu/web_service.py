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
from paddle_serving_server_gpu.pipeline.util import AvailablePortGenerator


class DefaultRpcServer(object):
    def __init__(self, available_port_generator):
        self.available_port_generator = available_port_generator
        self.gpus = None
        self.rpc_service_list = []
        self.server_pros = []
        self.port_list = []
        self.model_config = None
        self.workdir = None
        self.device = None

    def get_port_list(self):
        return self.port_list

    def load_model_config(self, model_config):
        self.model_config = model_config

    def set_gpus(self, gpus):
        self.gpus = [int(x) for x in gpus.split(",")]

    def _prepare_one_server(self,
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

    def _start_one_server(self, service_idx):
        self.rpc_service_list[service_idx].run_server()

    def prepare_server(self,
                       workdir="",
                       device="gpu",
                       mem_optim=True,
                       ir_optim=False):
        self.workdir = workdir
        self.device = device
        default_port = 12000
        while len(self.port_list) < len(self.gpus):
            self.port_list.append(self.available_port_generator.next())

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
                    self._prepare_one_server(
                        "{}_{}".format(self.workdir, i),
                        self.port_list[i],
                        gpuid,
                        thread_num=2,
                        mem_optim=mem_optim,
                        ir_optim=ir_optim))

    def start_server(self):
        import socket
        for i, service in enumerate(self.rpc_service_list):
            p = Process(target=self._start_one_server, args=(i, ))
            self.server_pros.append(p)
        for p in self.server_pros:
            p.start()


class DefaultPipelineServer(object):
    def __init__(self, available_port_generator):
        self.server = pipeline.PipelineServer()
        self.available_port_generator = available_port_generator

    def create_internel_op_class(self, f_preprocess, f_postprocess):
        class InternelOp(pipeline.Op):
            def init_op(self):
                pass

            def preprocess(self, input_dicts):
                (_, input_dict), = input_dicts.items()
                preped_data = f_preprocess(input_dict)
                return preped_data

            def postprocess(self, input_dicts, fetch_dict):
                (_, input_dict), = input_dicts.items()
                postped_data = f_postprocess(input_dict, fetch_dict)
                return postped_data

        return InternelOp

    def init_server(self,
                    internel_op_class,
                    internel_op_name,
                    internel_op_endpoints,
                    internel_op_client_config,
                    internel_op_concurrency,
                    internel_op_timeout=-1,
                    internel_op_retry=1,
                    internel_op_batch_size=1,
                    internel_op_auto_batching_timeout=None):
        read_op = pipeline.RequestOp()
        internel_op = internel_op_class(
            name=internel_op_name,
            input_ops=[read_op],
            server_endpoints=internel_op_endpoints,
            client_config=internel_op_client_config,
            concurrency=internel_op_concurrency,
            timeout=internel_op_timeout,
            retry=internel_op_retry,
            batch_size=internel_op_batch_size,
            auto_batching_timeout=internel_op_auto_batching_timeout)
        response_op = pipeline.ResponseOp(input_ops=[internel_op])
        self.server.set_response_op(response_op)

    def prepare_server(self,
                       rpc_port,
                       http_port,
                       worker_num,
                       build_dag_each_worker=False,
                       is_thread_op=False,
                       client_type="brpc",
                       retry=1,
                       use_profile=False,
                       tracer_interval_s=-1):
        default_server_conf = {
            "port": rpc_port,
            "worker_num": worker_num,
            "build_dag_each_worker": build_dag_each_worker,
            "grpc_gateway_port": http_port,
            "dag": {
                "is_thread_op": is_thread_op,
                "client_type": client_type,
                "retry": retry,
                "use_profile": use_profile,
                "tracer": {
                    "interval_s": tracer_interval_s,
                }
            }
        }
        self.server.prepare_server(yml_dict=default_server_conf)

    def start_server(self):
        self.server.run_server()


class PipelineWebService(object):
    def __init__(self, name="default"):
        self.name = name
        self.port = None
        self.model_config = None
        self.available_port_generator = AvailablePortGenerator(12000)
        self.default_rpc_server = DefaultRpcServer(
            self.available_port_generator)
        self.default_pipeline_server = DefaultPipelineServer(
            self.available_port_generator)

    def load_model_config(self, model_config):
        self.model_config = model_config
        self.default_rpc_server.load_model_config(model_config)

    def set_gpus(self, gpus):
        self.default_rpc_server.set_gpus(gpus)

    def prepare_server(self,
                       workdir="",
                       port=9393,
                       device="gpu",
                       worker_num=4,
                       mem_optim=True,
                       ir_optim=False):
        if not self.available_port_generator.port_is_available(port):
            raise SystemExit(
                "Failed to prepare server: prot({}) is not available".format(
                    port))
        self.port = port

        # rpc server
        self.default_rpc_server.prepare_server(
            workdir=workdir,
            device=device,
            mem_optim=mem_optim,
            ir_optim=ir_optim)
        rpc_endpoints = self.default_rpc_server.get_port_list()

        # pipeline server
        internel_op_class = self.default_pipeline_server.create_internel_op_class(
            self.preprocess, self.postprocess)
        internel_op_endpoints = [
            "127.0.0.1:{}".format(port) for port in rpc_endpoints
        ]
        self.default_pipeline_server.init_server(
            internel_op_class=internel_op_class,
            internel_op_name=self.name,
            internel_op_endpoints=internel_op_endpoints,
            internel_op_client_config="{}/serving_server_conf.prototxt".format(
                self.model_config),
            internel_op_concurrency=worker_num)
        self.default_pipeline_server.prepare_server(
            rpc_port=self.available_port_generator.next(),
            http_port=self.port,
            worker_num=worker_num)

    def run_service(self):
        import socket
        localIP = socket.gethostbyname(socket.gethostname())
        print("web service address: http://{}:{}/prediction"
              .format(localIP, self.port))
        self.default_rpc_server.start_server()
        self.default_pipeline_server.start_server()

    def preprocess(self, feed_dict):
        return feed_dict

    def postprocess(self, feed_dict, fetch_dict):
        return fetch_dict
