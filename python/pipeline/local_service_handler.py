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
from .error_catch import ErrorCatch, CustomException, CustomExceptionCode
#from paddle_serving_server import OpMaker, OpSeqMaker
#from paddle_serving_server import Server as GpuServer
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
                 device_type=-1,
                 devices="",
                 fetch_names=None,
                 mem_optim=True,
                 ir_optim=False,
                 available_port_generator=None,
                 use_profile=False,
                 precision="fp32",
                 use_mkldnn=False,
                 mkldnn_cache_capacity=0,
                 mkldnn_op_list=None,
                 mkldnn_bf16_op_list=None,
                 min_subgraph_size=3,
                 dynamic_shape_info={},
                 use_calib=False):
        """
        Initialization of localservicehandler

        Args:
           model_config: model config path
           client_type: brpc, grpc and local_predictor[default]
           workdir: work directory
           thread_num: number of threads, concurrent quantity.
           device_type: support multiple devices. -1=Not set, determined by
               `devices`. 0=cpu, 1=gpu, 2=tensorRT, 3=arm cpu, 4=kunlun xpu
           devices: gpu id list[gpu], "" default[cpu]
           fetch_names: get fetch names out of LocalServiceHandler in 
               local_predictor mode. fetch_names_ is compatible for Client().
           mem_optim: use memory/graphics memory optimization, True default.
           ir_optim: use calculation chart optimization, False default.
           available_port_generator: generate available ports
           use_profile: use profiling, False default.
           precision: inference precesion, e.g. "fp32", "fp16", "int8"
           use_mkldnn: use mkldnn, default False.
           mkldnn_cache_capacity: cache capacity of mkldnn, 0 means no limit.
           mkldnn_op_list: OP list optimized by mkldnn, None default.
           mkldnn_bf16_op_list: OP list optimized by mkldnn bf16, None default.
           use_calib: set inference use_calib_mode param, False default.

        Returns:
           None
        """
        if available_port_generator is None:
            available_port_generator = util.GetAvailablePortGenerator()

        self._model_config = model_config
        self._port_list = []
        self._device_name = "cpu"
        self._use_gpu = False
        self._use_trt = False
        self._use_lite = False
        self._use_xpu = False
        self._use_ascend_cl = False
        self._use_mkldnn = False
        self._mkldnn_cache_capacity = 0
        self._mkldnn_op_list = None
        self._mkldnn_bf16_op_list = None
        self.min_subgraph_size = 3
        self.dynamic_shape_info = {}
        self._use_calib = False

        if device_type == -1:
            # device_type is not set, determined by `devices`, 
            if devices == "":
                # CPU
                self._device_name = "cpu"
                devices = [-1]
            else:
                # GPU
                self._device_name = "gpu"
                self._use_gpu = True
                devices = [int(x) for x in devices.split(",")]

        elif device_type == 0:
            # CPU
            self._device_name = "cpu"
            devices = [-1]
        elif device_type == 1:
            # GPU
            self._device_name = "gpu"
            self._use_gpu = True
            devices = [int(x) for x in devices.split(",")]
        elif device_type == 2:
            # Nvidia Tensor RT
            self._device_name = "gpu"
            self._use_gpu = True
            devices = [int(x) for x in devices.split(",")]
            self._use_trt = True
            self.min_subgraph_size = min_subgraph_size
            self.dynamic_shape_info = dynamic_shape_info
        elif device_type == 3:
            # ARM CPU
            self._device_name = "arm"
            devices = [-1]
            self._use_lite = True
        elif device_type == 4:
            # Kunlun XPU
            self._device_name = "arm"
            devices = [int(x) for x in devices.split(",")]
            self._use_lite = True
            self._use_xpu = True
        elif device_type == 5:
            # Ascend 310 ARM CPU
            self._device_name = "arm"
            devices = [int(x) for x in devices.split(",")]
            self._use_lite = True
            self._use_ascend_cl = True
        elif device_type == 6:
            # Ascend 910 ARM CPU
            self._device_name = "arm"
            devices = [int(x) for x in devices.split(",")]
            self._use_ascend_cl = True
        else:
            _LOGGER.error(
                "LocalServiceHandler initialization fail. device_type={}"
                .format(device_type))

        if client_type == "brpc" or client_type == "grpc":
            for _ in devices:
                self._port_list.append(available_port_generator.next())
            _LOGGER.info("Create ports for devices:{}. Port:{}"
                         .format(devices, self._port_list))

        self._client_type = client_type
        self._workdir = workdir
        self._devices = devices
        self._thread_num = thread_num
        self._mem_optim = mem_optim
        self._ir_optim = ir_optim
        self._local_predictor_client = None
        self._rpc_service_list = []
        self._server_pros = []
        self._use_profile = use_profile
        self._fetch_names = fetch_names
        self._precision = precision
        self._use_mkldnn = use_mkldnn
        self._mkldnn_cache_capacity = mkldnn_cache_capacity
        self._mkldnn_op_list = mkldnn_op_list
        self._mkldnn_bf16_op_list = mkldnn_bf16_op_list
        self._use_calib = use_calib

        _LOGGER.info(
            "Models({}) will be launched by device {}. use_gpu:{}, "
            "use_trt:{}, use_lite:{}, use_xpu:{}, device_type:{}, devices:{}, "
            "mem_optim:{}, ir_optim:{}, use_profile:{}, thread_num:{}, "
            "client_type:{}, fetch_names:{}, precision:{}, use_calib:{}, "
            "use_mkldnn:{}, mkldnn_cache_capacity:{}, mkldnn_op_list:{}, "
            "mkldnn_bf16_op_list:{}, use_ascend_cl:{}, min_subgraph_size:{},"
            "is_set_dynamic_shape_info:{}".format(
                model_config, self._device_name, self._use_gpu, self._use_trt,
                self._use_lite, self._use_xpu, device_type, self._devices, self.
                _mem_optim, self._ir_optim, self._use_profile, self._thread_num,
                self._client_type, self._fetch_names, self._precision, self.
                _use_calib, self._use_mkldnn, self._mkldnn_cache_capacity, self.
                _mkldnn_op_list, self._mkldnn_bf16_op_list, self._use_ascend_cl,
                self.min_subgraph_size, bool(len(self.dynamic_shape_info))))

    def get_fetch_list(self):
        return self._fetch_names

    def get_port_list(self):
        return self._port_list

    def get_client(self, concurrency_idx):
        """
        Function get_client is only used for local predictor case, creates one
        LocalPredictor object, and initializes the paddle predictor by function
        load_model_config.The concurrency_idx is used to select running devices.  

        Args:
            concurrency_idx: process/thread index

        Returns:
            _local_predictor_client
        """

        #checking the legality of concurrency_idx.
        device_num = len(self._devices)
        if device_num <= 0:
            _LOGGER.error("device_num must be not greater than 0. devices({})".
                          format(self._devices))
            raise ValueError("The number of self._devices error")

        if concurrency_idx < 0:
            _LOGGER.error("concurrency_idx({}) must be one positive number".
                          format(concurrency_idx))
            concurrency_idx = 0
        elif concurrency_idx >= device_num:
            concurrency_idx = concurrency_idx % device_num

        _LOGGER.info("GET_CLIENT : concurrency_idx={}, device_num={}".format(
            concurrency_idx, device_num))
        from paddle_serving_app.local_predict import LocalPredictor
        if self._local_predictor_client is None:
            self._local_predictor_client = LocalPredictor()
            # load model config and init predictor
            self._local_predictor_client.load_model_config(
                model_path=self._model_config,
                use_gpu=self._use_gpu,
                gpu_id=self._devices[concurrency_idx],
                use_profile=self._use_profile,
                thread_num=self._thread_num,
                mem_optim=self._mem_optim,
                ir_optim=self._ir_optim,
                use_trt=self._use_trt,
                use_lite=self._use_lite,
                use_xpu=self._use_xpu,
                precision=self._precision,
                use_mkldnn=self._use_mkldnn,
                mkldnn_cache_capacity=self._mkldnn_cache_capacity,
                mkldnn_op_list=self._mkldnn_op_list,
                mkldnn_bf16_op_list=self._mkldnn_bf16_op_list,
                use_ascend_cl=self._use_ascend_cl,
                min_subgraph_size=self.min_subgraph_size,
                dynamic_shape_info=self.dynamic_shape_info,
                use_calib=self._use_calib)
        return self._local_predictor_client

    def get_client_config(self):
        return os.path.join(self._model_config, "serving_server_conf.prototxt")

    def _prepare_one_server(self, workdir, port, gpuid, thread_num, mem_optim,
                            ir_optim, precision):
        """
        According to self._device_name, generating one Cpu/Gpu/Arm Server, and
        setting the model config amd startup params.

        Args:
            workdir: work directory
            port: network port
            gpuid: gpu id
            thread_num: thread num
            mem_optim: use memory/graphics memory optimization
            ir_optim: use calculation chart optimization
            precision: inference precison, e.g."fp32", "fp16", "int8"

        Returns:
            server: CpuServer/GpuServer
        """
        if self._device_name == "cpu":
            from paddle_serving_server import OpMaker, OpSeqMaker, Server
            op_maker = OpMaker()
            read_op = op_maker.create('GeneralReaderOp')
            general_infer_op = op_maker.create('GeneralInferOp')
            general_response_op = op_maker.create('GeneralResponseOp')

            op_seq_maker = OpSeqMaker()
            op_seq_maker.add_op(read_op)
            op_seq_maker.add_op(general_infer_op)
            op_seq_maker.add_op(general_response_op)

            server = Server()
        else:
            #gpu or arm
            from paddle_serving_server import OpMaker, OpSeqMaker, Server
            op_maker = OpMaker()
            read_op = op_maker.create('GeneralReaderOp')
            general_infer_op = op_maker.create('GeneralInferOp')
            general_response_op = op_maker.create('GeneralResponseOp')

            op_seq_maker = OpSeqMaker()
            op_seq_maker.add_op(read_op)
            op_seq_maker.add_op(general_infer_op)
            op_seq_maker.add_op(general_response_op)

            server = Server()
            if gpuid >= 0:
                server.set_gpuid(gpuid)
            # TODO: support arm or arm + xpu later
            server.set_device(self._device_name)
            if self._use_xpu:
                server.set_xpu()
            if self._use_lite:
                server.set_lite()
            if self._use_ascend_cl:
                server.set_ascend_cl()

        server.set_op_sequence(op_seq_maker.get_op_sequence())
        server.set_num_threads(thread_num)
        server.set_memory_optimize(mem_optim)
        server.set_ir_optimize(ir_optim)
        server.set_precision(precision)

        server.load_model_config(self._model_config)
        server.prepare_server(
            workdir=workdir, port=port, device=self._device_name)
        if self._fetch_names is None:
            self._fetch_names = server.get_fetch_list()
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
                    ir_optim=self._ir_optim,
                    precision=self._precision))

    def start_server(self):
        """
        Start multiple processes and start one server in each process
        """
        for i, _ in enumerate(self._rpc_service_list):
            p = multiprocessing.Process(
                target=self._start_one_server, args=(i, ))
            p.daemon = True
            self._server_pros.append(p)
        for p in self._server_pros:
            p.start()
