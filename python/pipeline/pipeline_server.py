#   Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
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
from concurrent import futures
import grpc
import logging
import json
import socket
import contextlib
from contextlib import closing
import multiprocessing
import yaml
import io
import time
import os
from .error_catch import ErrorCatch, CustomException, CustomExceptionCode, ParamChecker, ParamVerify
from .proto import pipeline_service_pb2_grpc, pipeline_service_pb2
from . import operator
from . import dag
from . import util
from . import channel
from paddle_serving_server.env import CONF_HOME
from paddle_serving_server.util import dump_pid_file

_LOGGER = logging.getLogger(__name__)


class PipelineServicer(pipeline_service_pb2_grpc.PipelineServiceServicer):
    """
    Pipeline Servicer entrance.
    """
    def __init__(self, name, response_op, dag_conf, worker_idx=-1):

        @ErrorCatch
        @ParamChecker
        def init_helper(self, name, response_op, 
          dag_conf: dict,
          worker_idx=-1):
           self._name = name
           self._dag_executor = dag.DAGExecutor(response_op, dag_conf, worker_idx)
           self._dag_executor.start()
            
        super(PipelineServicer, self).__init__()
        init_res = init_helper(self, name, response_op, dag_conf, worker_idx)
        if init_res[1].err_no != CustomExceptionCode.OK.value :
            raise CustomException(CustomExceptionCode.INIT_ERROR, "pipeline server init error")
        print("[PipelineServicer] succ init")
        _LOGGER.info("[PipelineServicer] succ init")

    def inference(self, request, context):
        _LOGGER.info(
            "(log_id={}) inference request name:{} self.name:{} time:{}".format(
                request.logid, request.name, self._name, time.time()))
        if request.name != "" and request.name != self._name:
            _LOGGER.error("(log_id={}) name dismatch error. request.name:{},"
                          "server.name={}".format(request.logid, request.name,
                                                  self._name))
            resp = pipeline_service_pb2.Response()
            resp.err_no = channel.ChannelDataErrcode.NO_SERVICE.value
            resp.err_msg = "Failed to inference: Service name error."
            return resp
        resp = self._dag_executor.call(request)
        return resp


@contextlib.contextmanager
def _reserve_port(port):
    """
    Find and reserve a port for all subprocesses to use.
    """
    sock = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
    if sock.getsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT) == 0:
        raise RuntimeError("Failed to set SO_REUSEPORT.")
    sock.bind(('', port))
    try:
        yield sock.getsockname()[1]
    finally:
        sock.close()

class PipelineServer(object):
    """
    Pipeline Server : grpc gateway + grpc server.
    """

    def __init__(self, name=None):
        self._name = name  # for grpc-gateway path
        self._rpc_port = None
        self._worker_num = None
        self._response_op = None
        self._proxy_server = None

    def _grpc_gateway(self, grpc_port, http_port):
        """
        Running a gateway server, linking libproxy_server.so

        Args:
            grpc_port: GRPC port
            http_port: HTTP port

        Returns:
            None
        """
        import os
        from ctypes import cdll
        from . import gateway
        lib_path = os.path.join(
            os.path.dirname(gateway.__file__), "libproxy_server.so")
        proxy_server = cdll.LoadLibrary(lib_path)
        proxy_server.run_proxy_server(grpc_port, http_port)

    def _run_grpc_gateway(self, grpc_port, http_port):
        """
        Starting the GRPC gateway in a new process. Exposing one 
        available HTTP port outside, and reflecting the data to RPC port.

        Args:
            grpc_port: GRPC port
            http_port: HTTP port

        Returns:
            None
        """
        if http_port <= 0:
            _LOGGER.info("Ignore grpc_gateway configuration.")
            return
        if not util.AvailablePortGenerator.port_is_available(http_port):
            raise SystemExit("Failed to run grpc-gateway: prot {} "
                             "is already used".format(http_port))
        if self._proxy_server is not None:
            raise RuntimeError("Proxy server has been started.")
        self._proxy_server = multiprocessing.Process(
            target=self._grpc_gateway, args=(
                grpc_port,
                http_port, ))
        self._proxy_server.daemon = True
        self._proxy_server.start()

    def set_response_op(self, response_op):
        """
        Set the response OP.

        Args:
            response_op: ResponseOp or its subclass object

        Returns:
            None
        """
        if not isinstance(response_op, operator.ResponseOp):
            raise Exception("Failed to set response_op: response_op "
                            "must be ResponseOp type.")
        if len(response_op.get_input_ops()) != 1:
            raise Exception("Failed to set response_op: response_op "
                            "can only have one previous op.")
        self._response_op = response_op
        self._used_op, _ = dag.DAG.get_use_ops(self._response_op)

    def prepare_server(self, yml_file=None, yml_dict=None):
        """
        Reading configures from the yml file(config.yaml), and launching
        local services.

        Args:
            yml_file: Reading configures from yaml files
            yml_dict: Reading configures from yaml dict.
   
        Returns:
            None 
        """
        conf = ServerYamlConfChecker.load_server_yaml_conf(
            yml_file=yml_file, yml_dict=yml_dict)

        self._rpc_port = conf.get("rpc_port")
        self._http_port = conf.get("http_port")
        if self._rpc_port is None:
            if self._http_port is None:
                raise SystemExit("Failed to prepare_server: rpc_port or "
                                 "http_port can not be None.")
            else:
                # http mode: generate rpc_port
                if not util.AvailablePortGenerator.port_is_available(
                        self._http_port):
                    raise SystemExit("Failed to prepare_server: http_port({}) "
                                     "is already used".format(self._http_port))
                self._rpc_port = util.GetAvailablePortGenerator().next()
        else:
            if not util.AvailablePortGenerator.port_is_available(
                    self._rpc_port):
                raise SystemExit("Failed to prepare_server: prot {} "
                                 "is already used".format(self._rpc_port))
            if self._http_port is None:
                # rpc mode
                pass
            else:
                # http mode
                if not util.AvailablePortGenerator.port_is_available(
                        self._http_port):
                    raise SystemExit("Failed to prepare_server: http_port({}) "
                                     "is already used".format(self._http_port))
        # write the port info into ProcessInfo.json
        portList = []
        if self._http_port is not None:
            portList.append(self._rpc_port)
        if self._rpc_port is not None:
            portList.append(self._http_port)
        if len(portList):
            dump_pid_file(portList, "pipline")
        self._worker_num = conf["worker_num"]
        self._build_dag_each_worker = conf["build_dag_each_worker"]
        self._init_ops(conf["op"])

        _LOGGER.info("============= PIPELINE SERVER =============")
        _LOGGER.info("\n{}".format(
            json.dumps(
                conf, indent=4, separators=(',', ':'))))
        if self._build_dag_each_worker is True:
            _LOGGER.warning(
                "(Make sure that install grpcio whl with --no-binary flag: "
                "pip install grpcio --no-binary grpcio)")
        _LOGGER.info("-------------------------------------------")

        self._conf = conf
        self._start_local_rpc_service()

    def _init_ops(self, op_conf):
        """
        Initializing all OPs from dicetory.

        Args:
            op_conf: the op configures in yaml dict.

        Returns:
            None.
        """
        default_conf = {
            "concurrency": 1,
            "timeout": -1,
            "retry": 1,
            "batch_size": 1,
            "auto_batching_timeout": -1,
            "local_service_conf": {
                "workdir": "",
                "thread_num": 2,
                "device_type": -1,
                "devices": "",
                "mem_optim": True,
                "ir_optim": False,
                "precision": "fp32",
                "use_calib": False,
                "use_mkldnn": False,
                "mkldnn_cache_capacity": 0,
                "min_subgraph_size": 3,
            },
        }
        for op in self._used_op:
            if not isinstance(op, operator.RequestOp) and not isinstance(
                    op, operator.ResponseOp):
                conf = op_conf.get(op.name, default_conf)
                op.init_from_dict(conf)

    def _start_local_rpc_service(self):
        # only brpc now
        if self._conf["dag"]["client_type"] != "brpc":
            _LOGGER.warning("Local service version must be brpc type now.")
        for op in self._used_op:
            if not isinstance(op, operator.RequestOp):
                op.launch_local_rpc_service()

    def run_server(self):
        """
        If _build_dag_each_worker is True, Starting _worker_num processes and 
        running one GRPC server in each process. Otherwise, Staring one GRPC
        server.

        Args:
            None

        Returns:
            None
        """
        if self._build_dag_each_worker:
            with _reserve_port(self._rpc_port) as port:
                bind_address = 'localhost:{}'.format(port)
                workers = []
                for i in range(self._worker_num):
                    worker = multiprocessing.Process(
                        target=self._run_server_func,
                        args=(bind_address, self._response_op, self._conf, i))
                    worker.start()
                    workers.append(worker)
                self._run_grpc_gateway(
                    grpc_port=self._rpc_port,
                    http_port=self._http_port)  # start grpc_gateway
                for worker in workers:
                    worker.join()
        else:
            server = grpc.server(
                futures.ThreadPoolExecutor(max_workers=self._worker_num),
                options=[('grpc.max_send_message_length', 256 * 1024 * 1024),
                         ('grpc.max_receive_message_length', 256 * 1024 * 1024)
                         ])
            pipeline_service_pb2_grpc.add_PipelineServiceServicer_to_server(
                PipelineServicer(self._name, self._response_op, self._conf),
                server)
            server.add_insecure_port('[::]:{}'.format(self._rpc_port))
            server.start()
            self._run_grpc_gateway(
                grpc_port=self._rpc_port,
                http_port=self._http_port)  # start grpc_gateway
            server.wait_for_termination()

    def _run_server_func(self, bind_address, response_op, dag_conf, worker_idx):
        """
        Running one GRPC server with PipelineServicer.

        Args:
            bind_address: binding IP/Port
            response_op: ResponseOp or its subclass object
            dag_conf: DAG config
            worker_idx: Process index.
        """
        options = [('grpc.so_reuseport', 1),
                   ('grpc.max_send_message_length', 256 * 1024 * 1024),
                   ('grpc.max_send_message_length', 256 * 1024 * 1024)]
        server = grpc.server(
            futures.ThreadPoolExecutor(
                max_workers=1, ), options=options)
        pipeline_service_pb2_grpc.add_PipelineServiceServicer_to_server(
            PipelineServicer(self._name, response_op, dag_conf, worker_idx),
            server)
        server.add_insecure_port(bind_address)
        server.start()
        server.wait_for_termination()


class ServerYamlConfChecker(object):
    """
    Checking validities of server yaml files.
    """

    def __init__(self):
        pass

    @staticmethod
    def load_server_yaml_conf(yml_file=None, yml_dict=None):
        if yml_file is not None and yml_dict is not None:
            raise SystemExit("Failed to prepare_server: only one of yml_file"
                             " or yml_dict can be selected as the parameter.")
        if yml_file is not None:
            with io.open(yml_file, encoding='utf-8') as f:
                conf = yaml.load(f.read(), yaml.FullLoader)
        elif yml_dict is not None:
            conf = yml_dict
        else:
            raise SystemExit("Failed to prepare_server: yml_file or yml_dict"
                             " can not be None.")
        ServerYamlConfChecker.check_server_conf(conf)
        ServerYamlConfChecker.check_dag_conf(conf["dag"])
        ServerYamlConfChecker.check_tracer_conf(conf["dag"]["tracer"])
        for op_name in conf["op"]:
            ServerYamlConfChecker.check_op_conf(conf["op"][op_name])
            ServerYamlConfChecker.check_local_service_conf(conf["op"][op_name][
                "local_service_conf"])
        return conf

    @staticmethod
    def check_conf(conf, default_conf, conf_type, conf_qualification):
        ServerYamlConfChecker.fill_with_default_conf(conf, default_conf)
        ServerYamlConfChecker.check_conf_type(conf, conf_type)
        ServerYamlConfChecker.check_conf_qualification(conf, conf_qualification)

    @staticmethod
    def check_server_conf(conf):
        default_conf = {
            # "rpc_port": 9292,
            "worker_num": 1,
            "build_dag_each_worker": False,
            #"http_port": 0,
            "dag": {},
            "op": {},
        }

        conf_type = {
            "rpc_port": int,
            "http_port": int,
            "worker_num": int,
            "build_dag_each_worker": bool,
            "grpc_gateway_port": int,
        }

        conf_qualification = {
            "rpc_port": [(">=", 1024), ("<=", 65535)],
            "http_port": [(">=", 1024), ("<=", 65535)],
            "worker_num": (">=", 1),
        }

        ServerYamlConfChecker.check_conf(conf, default_conf, conf_type,
                                         conf_qualification)

    @staticmethod
    def check_local_service_conf(conf):
        default_conf = {
            "workdir": "",
            "thread_num": 2,
            "device_type": -1,
            "devices": "",
            "mem_optim": True,
            "ir_optim": False,
            "precision": "fp32",
            "use_calib": False,
            "use_mkldnn": False,
            "mkldnn_cache_capacity": 0,
            "min_subgraph_size": 3,
        }
        conf_type = {
            "model_config": str,
            "workdir": str,
            "thread_num": int,
            "device_type": int,
            "devices": str,
            "mem_optim": bool,
            "ir_optim": bool,
            "precision": str,
            "use_calib": bool,
            "use_mkldnn": bool,
            "mkldnn_cache_capacity": int,
            "mkldnn_op_list": list,
            "mkldnn_bf16_op_list": list,
            "min_subgraph_size": int,
        }
        conf_qualification = {"thread_num": (">=", 1), }
        ServerYamlConfChecker.check_conf(conf, default_conf, conf_type,
                                         conf_qualification)

    @staticmethod
    def check_op_conf(conf):
        default_conf = {
            "concurrency": 1,
            "timeout": -1,
            "retry": 1,
            "batch_size": 1,
            "auto_batching_timeout": -1,
            "local_service_conf": {},
        }
        conf_type = {
            "server_endpoints": list,
            "fetch_list": list,
            "client_config": str,
            "concurrency": int,
            "timeout": int,
            "retry": int,
            "batch_size": int,
            "auto_batching_timeout": int,
        }
        conf_qualification = {
            "concurrency": (">=", 1),
            "retry": (">=", 1),
            "batch_size": (">=", 1),
        }
        ServerYamlConfChecker.check_conf(conf, default_conf, conf_type,
                                         conf_qualification)

    @staticmethod
    def check_tracer_conf(conf):
        default_conf = {"interval_s": -1, }

        conf_type = {"interval_s": int, }

        conf_qualification = {}

        ServerYamlConfChecker.check_conf(conf, default_conf, conf_type,
                                         conf_qualification)

    @staticmethod
    def check_dag_conf(conf):
        default_conf = {
            "retry": 1,
            "client_type": "brpc",
            "use_profile": False,
            "channel_size": 0,
            "is_thread_op": True,
            "tracer": {},
            "channel_recv_frist_arrive": False,
        }

        conf_type = {
            "retry": int,
            "client_type": str,
            "use_profile": bool,
            "channel_size": int,
            "is_thread_op": bool,
            "channel_recv_frist_arrive": bool,
        }

        conf_qualification = {
            "retry": (">=", 1),
            "client_type": ("in", ["brpc", "grpc"]),
            "channel_size": (">=", 0),
        }

        ServerYamlConfChecker.check_conf(conf, default_conf, conf_type,
                                         conf_qualification)

    @staticmethod
    def fill_with_default_conf(conf, default_conf):
        for key, val in default_conf.items():
            if conf.get(key) is None:
                _LOGGER.warning("[CONF] {} not set, use default: {}"
                                .format(key, val))
                conf[key] = val

    @staticmethod
    def check_conf_type(conf, conf_type):
        for key, val in conf_type.items():
            if key not in conf:
                continue
            if not isinstance(conf[key], val):
                raise SystemExit("[CONF] {} must be {} type, but get {}."
                                 .format(key, val, type(conf[key])))

    @staticmethod
    def check_conf_qualification(conf, conf_qualification):
        for key, qualification in conf_qualification.items():
            if key not in conf:
                continue
            if not isinstance(qualification, list):
                qualification = [qualification]
            if not ServerYamlConfChecker.qualification_check(conf[key],
                                                             qualification):
                raise SystemExit("[CONF] {} must be {}, but get {}."
                                 .format(key, ", ".join([
                                     "{} {}"
                                     .format(q[0], q[1]) for q in qualification
                                 ]), conf[key]))

    @staticmethod
    def qualification_check(value, qualifications):
        if not isinstance(qualifications, list):
            qualifications = [qualifications]
        ok = True
        for q in qualifications:
            operator, limit = q
            if operator == "<":
                ok = value < limit
            elif operator == "==":
                ok = value == limit
            elif operator == ">":
                ok = value > limit
            elif operator == "<=":
                ok = value <= limit
            elif operator == ">=":
                ok = value >= limit
            elif operator == "in":
                ok = value in limit
            else:
                raise SystemExit("unknow operator: {}".format(operator))
            if ok == False:
                break
        return ok
