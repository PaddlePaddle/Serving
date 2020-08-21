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

from .proto import pipeline_service_pb2_grpc
from .operator import ResponseOp, RequestOp
from .dag import DAGExecutor, DAG
from .util import AvailablePortGenerator

_LOGGER = logging.getLogger(__name__)


class PipelineServicer(pipeline_service_pb2_grpc.PipelineServiceServicer):
    def __init__(self, response_op, dag_conf, worker_idx=-1):
        super(PipelineServicer, self).__init__()
        # init dag executor
        self._dag_executor = DAGExecutor(response_op, dag_conf, worker_idx)
        self._dag_executor.start()
        _LOGGER.info("[PipelineServicer] succ init")

    def inference(self, request, context):
        resp = self._dag_executor.call(request)
        return resp


@contextlib.contextmanager
def _reserve_port(port):
    """Find and reserve a port for all subprocesses to use."""
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
    def __init__(self):
        self._port = None
        self._worker_num = None
        self._response_op = None
        self._proxy_server = None

    def _grpc_gateway(self, grpc_port, http_port):
        import os
        from ctypes import cdll
        from . import gateway
        lib_path = os.path.join(
            os.path.dirname(gateway.__file__), "libproxy_server.so")
        proxy_server = cdll.LoadLibrary(lib_path)
        proxy_server.run_proxy_server(grpc_port, http_port)

    def _run_grpc_gateway(self, grpc_port, http_port):
        if http_port <= 0:
            _LOGGER.info("Ignore grpc_gateway configuration.")
            return
        if not AvailablePortGenerator.port_is_available(http_port):
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
        if not isinstance(response_op, ResponseOp):
            raise Exception("Failed to set response_op: response_op "
                            "must be ResponseOp type.")
        if len(response_op.get_input_ops()) != 1:
            raise Exception("Failed to set response_op: response_op "
                            "can only have one previous op.")
        self._response_op = response_op

    def prepare_server(self, yml_file=None, yml_dict=None):
        conf = ServerYamlConfChecker.load_server_yaml_conf(
            yml_file=yml_file, yml_dict=yml_dict)

        self._port = conf["port"]
        if not AvailablePortGenerator.port_is_available(self._port):
            raise SystemExit("Failed to prepare_server: prot {} "
                             "is already used".format(self._port))
        self._worker_num = conf["worker_num"]
        self._grpc_gateway_port = conf["grpc_gateway_port"]
        self._build_dag_each_worker = conf["build_dag_each_worker"]

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

    def start_local_rpc_service(self):
        # only brpc now
        if self._conf["dag"]["client_type"] != "brpc":
            raise ValueError("Local Servoce Version must be brpc type now.")
        used_op, _ = DAG.get_use_ops(self._response_op)
        for op in used_op:
            if not isinstance(op, RequestOp):
                op.launch_local_rpc_service()

    def run_server(self):
        if self._build_dag_each_worker:
            with _reserve_port(self._port) as port:
                bind_address = 'localhost:{}'.format(port)
                workers = []
                for i in range(self._worker_num):
                    show_info = (i == 0)
                    worker = multiprocessing.Process(
                        target=self._run_server_func,
                        args=(bind_address, self._response_op, self._conf, i))
                    worker.start()
                    workers.append(worker)
                self._run_grpc_gateway(
                    grpc_port=self._port,
                    http_port=self._grpc_gateway_port)  # start grpc_gateway
                for worker in workers:
                    worker.join()
        else:
            server = grpc.server(
                futures.ThreadPoolExecutor(max_workers=self._worker_num),
                options=[('grpc.max_send_message_length', 256 * 1024 * 1024),
                         ('grpc.max_receive_message_length', 256 * 1024 * 1024)
                         ])
            pipeline_service_pb2_grpc.add_PipelineServiceServicer_to_server(
                PipelineServicer(self._response_op, self._conf), server)
            server.add_insecure_port('[::]:{}'.format(self._port))
            server.start()
            self._run_grpc_gateway(
                grpc_port=self._port,
                http_port=self._grpc_gateway_port)  # start grpc_gateway
            server.wait_for_termination()

    def _run_server_func(self, bind_address, response_op, dag_conf, worker_idx):
        options = [('grpc.so_reuseport', 1),
                   ('grpc.max_send_message_length', 256 * 1024 * 1024),
                   ('grpc.max_send_message_length', 256 * 1024 * 1024)]
        server = grpc.server(
            futures.ThreadPoolExecutor(
                max_workers=1, ), options=options)
        pipeline_service_pb2_grpc.add_PipelineServiceServicer_to_server(
            PipelineServicer(response_op, dag_conf, worker_idx), server)
        server.add_insecure_port(bind_address)
        server.start()
        server.wait_for_termination()


class ServerYamlConfChecker(object):
    def __init__(self):
        pass

    @staticmethod
    def load_server_yaml_conf(yml_file=None, yml_dict=None):
        if yml_file is not None and yml_dict is not None:
            raise SystemExit("Failed to prepare_server: only one of yml_file"
                             " or yml_dict can be selected as the parameter.")
        if yml_file is not None:
            with open(yml_file) as f:
                conf = yaml.load(f.read())
        elif yml_dict is not None:
            conf = yml_dict
        else:
            raise SystemExit("Failed to prepare_server: yml_file or yml_dict"
                             " can not be None.")
        ServerYamlConfChecker.check_server_conf(conf)
        ServerYamlConfChecker.check_dag_conf(conf["dag"])
        ServerYamlConfChecker.check_tracer_conf(conf["dag"]["tracer"])
        return conf

    @staticmethod
    def check_conf(conf, default_conf, conf_type, conf_qualification):
        ServerYamlConfChecker.fill_with_default_conf(conf, default_conf)
        ServerYamlConfChecker.check_conf_type(conf, conf_type)
        ServerYamlConfChecker.check_conf_qualification(conf, conf_qualification)

    @staticmethod
    def check_server_conf(conf):
        default_conf = {
            "port": 9292,
            "worker_num": 1,
            "build_dag_each_worker": False,
            "grpc_gateway_port": 0,
            "dag": {},
        }

        conf_type = {
            "port": int,
            "worker_num": int,
            "build_dag_each_worker": bool,
            "grpc_gateway_port": int,
        }

        conf_qualification = {
            "port": [(">=", 1024), ("<=", 65535)],
            "worker_num": (">=", 1),
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
        }

        conf_type = {
            "retry": int,
            "client_type": str,
            "use_profile": bool,
            "channel_size": int,
            "is_thread_op": bool,
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
            if not isinstance(conf[key], val):
                raise SystemExit("[CONF] {} must be {} type, but get {}."
                                 .format(key, val, type(conf[key])))

    @staticmethod
    def check_conf_qualification(conf, conf_qualification):
        for key, qualification in conf_qualification.items():
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
