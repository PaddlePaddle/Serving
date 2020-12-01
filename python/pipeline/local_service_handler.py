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
#from paddle_serving_server_gpu import OpMaker, OpSeqMaker
#from paddle_serving_server_gpu import Server as GpuServer
#from paddle_serving_server import Server as CpuServer
from . import util
#from paddle_serving_app.local_predict import LocalPredictor

_LOGGER = logging.getLogger(__name__)
_workdir_name_gen = util.NameGenerator("workdir_")


class LocalServiceHandler(object):
    """
    LocalServiceHandler is the processor of the local service, contains
    three client types, brpc, grpc and local_predictor.If you use the 
    brpc or grpc, serveing startup ability is provided.If you use
    local_predictor, local predict ability is provided by paddle_serving_app.
    """

    def __init__(self,
                 model_config,
                 client_type='local_predictor',
                 workdir="",
                 thread_num=2,
                 devices="",
                 fetch_names=None,
                 mem_optim=True,
                 ir_optim=False,
                 available_port_generator=None,
                 use_trt=False,
                 use_profile=False):
        """
        Initialization of localservicehandler

        Args:
           model_config: model config path
           client_type: brpc, grpc and local_predictor[default]
           workdir: work directory
           thread_num: number of threads, concurrent quantity.
           devices: gpu id list[gpu], "" default[cpu]
           fetch_names: get fetch names out of LocalServiceHandler in 
               local_predictor mode. fetch_names_ is compatible for Client().
           mem_optim: use memory/graphics memory optimization, True default.
           ir_optim: use calculation chart optimization, False default.
           available_port_generator: generate available ports
           use_trt: use nvidia tensorRt engine, False default.
           use_profile: use profiling, False default.

        Returns:
           None
        """
        if available_port_generator is None:
            available_port_generator = util.GetAvailablePortGenerator()

        self._model_config = model_config
        self._port_list = []
        self._device_type = "cpu"
        if devices == "":
            # cpu
            devices = [-1]
            self._device_type = "cpu"
            self._port_list.append(available_port_generator.next())
            _LOGGER.info("Model({}) will be launch in cpu device. Port({})"
                         .format(model_config, self._port_list))
        else:
            # gpu
            self._device_type = "gpu"
            devices = [int(x) for x in devices.split(",")]
            for _ in devices:
                self._port_list.append(available_port_generator.next())
            _LOGGER.info("Model({}) will be launch in gpu device: {}. Port({})"
                         .format(model_config, devices, self._port_list))
        self._client_type = client_type
        self._workdir = workdir
        self._devices = devices
        self._thread_num = thread_num
        self._mem_optim = mem_optim
        self._ir_optim = ir_optim
        self._local_predictor_client = None
        self._rpc_service_list = []
        self._server_pros = []
        self._use_trt = use_trt
        self._use_profile = use_profile
        self.fetch_names_ = fetch_names

    def get_fetch_list(self):
        return self.fetch_names_

    def get_port_list(self):
        return self._port_list

    def get_client(self):
        """
        Function get_client is only used for local predictor case, creates one
        LocalPredictor object, and initializes the paddle predictor by function
        load_model_config.

        Args:
            None

        Returns:
            _local_predictor_client
        """
        from paddle_serving_app.local_predict import LocalPredictor
        if self._local_predictor_client is None:
            self._local_predictor_client = LocalPredictor()
            use_gpu = False
            if self._device_type == "gpu":
                use_gpu = True
            self._local_predictor_client.load_model_config(
                model_path=self._model_config,
                use_gpu=use_gpu,
                gpu_id=self._devices[0],
                use_profile=self._use_profile,
                thread_num=self._thread_num,
                mem_optim=self._mem_optim,
                ir_optim=self._ir_optim,
                use_trt=self._use_trt)
        return self._local_predictor_client

    def get_client_config(self):
        return os.path.join(self._model_config, "serving_server_conf.prototxt")

    def _prepare_one_server(self, workdir, port, gpuid, thread_num, mem_optim,
                            ir_optim):
        """
        According to _device_type, generating one CpuServer or GpuServer, and
        setting the model config amd startup params.

        Args:
            workdir: work directory
            port: network port
            gpuid: gpu id
            thread_num: thread num
            mem_optim: use memory/graphics memory optimization
            ir_optim: use calculation chart optimization

        Returns:
            server: CpuServer/GpuServer
        """
        if self._device_type == "cpu":
            from paddle_serving_server import OpMaker, OpSeqMaker, Server
            op_maker = OpMaker()
            read_op = op_maker.create('general_reader')
            general_infer_op = op_maker.create('general_infer')
            general_response_op = op_maker.create('general_response')

            op_seq_maker = OpSeqMaker()
            op_seq_maker.add_op(read_op)
            op_seq_maker.add_op(general_infer_op)
            op_seq_maker.add_op(general_response_op)

            server = Server()
        else:
            #gpu
            from paddle_serving_server_gpu import OpMaker, OpSeqMaker, Server
            op_maker = OpMaker()
            read_op = op_maker.create('general_reader')
            general_infer_op = op_maker.create('general_infer')
            general_response_op = op_maker.create('general_response')

            op_seq_maker = OpSeqMaker()
            op_seq_maker.add_op(read_op)
            op_seq_maker.add_op(general_infer_op)
            op_seq_maker.add_op(general_response_op)

            server = Server()
            if gpuid >= 0:
                server.set_gpuid(gpuid)

        server.set_op_sequence(op_seq_maker.get_op_sequence())
        server.set_num_threads(thread_num)
        server.set_memory_optimize(mem_optim)
        server.set_ir_optimize(ir_optim)

        server.load_model_config(self._model_config)
        server.prepare_server(
            workdir=workdir, port=port, device=self._device_type)
        if self.fetch_names_ is None:
            self.fetch_names_ = server.get_fetch_list()
        return server

    def _start_one_server(self, service_idx):
        """
        Start one server
     
        Args:
            service_idx: server index
 
        Returns:
            None
        """
        self._rpc_service_list[service_idx].run_server()

    def prepare_server(self):
        """
        Prepare all servers to be started, and append them into list. 
        """
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
        """
        Start multiple processes and start one server in each process
        """
        for i, service in enumerate(self._rpc_service_list):
            p = multiprocessing.Process(
                target=self._start_one_server, args=(i, ))
            p.daemon = True
            self._server_pros.append(p)
        for p in self._server_pros:
            p.start()
