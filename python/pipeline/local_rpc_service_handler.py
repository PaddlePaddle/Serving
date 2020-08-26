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

import os
import logging
import multiprocessing
try:
    from paddle_serving_server_gpu import OpMaker, OpSeqMaker, Server
    PACKAGE_VERSION = "GPU"
except ImportError:
    from paddle_serving_server import OpMaker, OpSeqMaker, Server
    PACKAGE_VERSION = "CPU"
import util

_LOGGER = logging.getLogger(__name__)
_workdir_name_gen = util.NameGenerator("workdir_")


class LocalRpcServiceHandler(object):
    def __init__(self,
                 model_config,
                 workdir="",
                 thread_num=2,
                 devices="",
                 mem_optim=True,
                 ir_optim=False,
                 available_port_generator=None):
        if available_port_generator is None:
            available_port_generator = util.GetAvailablePortGenerator()

        self._model_config = model_config
        self._port_list = []
        if devices == "":
            # cpu
            devices = [-1]
            self._port_list.append(available_port_generator.next())
            _LOGGER.info("Model({}) will be launch in cpu device. Port({})"
                         .format(model_config, self._port_list))
        else:
            # gpu
            if PACKAGE_VERSION == "CPU":
                raise ValueError(
                    "You are using the CPU version package("
                    "paddle-serving-server), unable to set devices")
            devices = [int(x) for x in devices.split(",")]
            for _ in devices:
                self._port_list.append(available_port_generator.next())
            _LOGGER.info("Model({}) will be launch in gpu device: {}. Port({})"
                         .format(model_config, devices, self._port_list))
        self._workdir = workdir
        self._devices = devices
        self._thread_num = thread_num
        self._mem_optim = mem_optim
        self._ir_optim = ir_optim

        self._rpc_service_list = []
        self._server_pros = []
        self._fetch_vars = None

    def get_fetch_list(self):
        return self._fetch_vars

    def get_port_list(self):
        return self._port_list

    def get_client_config(self):
        return os.path.join(self._model_config, "serving_server_conf.prototxt")

    def _prepare_one_server(self, workdir, port, gpuid, thread_num, mem_optim,
                            ir_optim):
        device = "gpu"
        if gpuid == -1:
            device = "cpu"
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
        server.set_num_threads(thread_num)
        server.set_memory_optimize(mem_optim)
        server.set_ir_optimize(ir_optim)

        server.load_model_config(self._model_config)
        if gpuid >= 0:
            server.set_gpuid(gpuid)
        server.prepare_server(workdir=workdir, port=port, device=device)
        if self._fetch_vars is None:
            self._fetch_vars = server.get_fetch_list()
        return server

    def _start_one_server(self, service_idx):
        self._rpc_service_list[service_idx].run_server()

    def prepare_server(self):
        for i, device_id in enumerate(self._devices):
            if self._workdir != "":
                workdir = "{}_{}".format(self._workdir, i)
            else:
                workdir = _workdir_name_gen.next()
            self._rpc_service_list.append(
                self._prepare_one_server(
                    workdir,
                    self._port_list[i],
                    device_id,
                    thread_num=self._thread_num,
                    mem_optim=self._mem_optim,
                    ir_optim=self._ir_optim))

    def start_server(self):
        for i, service in enumerate(self._rpc_service_list):
            p = multiprocessing.Process(
                target=self._start_one_server, args=(i, ))
            p.daemon = True
            self._server_pros.append(p)
        for p in self._server_pros:
            p.start()


class LocalPredictorServiceHandler(LocalRpcServiceHandler):
    def prepare_server(self):
        from paddle_serving_app.local_predict import Debugger
        gpuid = self._devices
        if gpuid == -1:
            gpu = False
        else:
            gpu = True
        self.predictor = Debugger()
        self.predictor.load_model_config(model_path=self._model_config, gpu=gpu, profile=False, cpu_num=1)

    def get_client(self):
        if self.predictor is None:
            raise ValueError("local predictor not yet created.")
        return self.predictor

    def get_fetch_list(self):
        if self.predictor is None:
            raise ValueError("local predictor not yet created.")
        return self.predictor.fetch_names_

    def start_server(self):
        pass
