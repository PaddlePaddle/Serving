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
import socket
import contextlib
from contextlib import closing
import multiprocessing
import yaml

from .proto import pipeline_service_pb2_grpc
from .operator import ResponseOp
from .dag import DAGExecutor

_LOGGER = logging.getLogger()


class PipelineServicer(pipeline_service_pb2_grpc.PipelineServiceServicer):
    def __init__(self, response_op, dag_config, show_info):
        super(PipelineServicer, self).__init__()
        # init dag executor
        self._dag_executor = DAGExecutor(
            response_op, dag_config, show_info=show_info)
        self._dag_executor.start()
        _LOGGER.info("[PipelineServicer] succ init")

    def inference(self, request, context):
        resp = self._dag_executor.call(request)
        return resp

    def __del__(self):
        self._dag_executor.stop()


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

    def set_response_op(self, response_op):
        if not isinstance(response_op, ResponseOp):
            raise Exception("response_op must be ResponseOp type.")
        if len(response_op.get_input_ops()) != 1:
            raise Exception("response_op can only have one previous op.")
        self._response_op = response_op

    def _port_is_available(self, port):
        with closing(socket.socket(socket.AF_INET, socket.SOCK_STREAM)) as sock:
            sock.settimeout(2)
            result = sock.connect_ex(('0.0.0.0', port))
        return result != 0

    def prepare_server(self, yml_file):
        with open(yml_file) as f:
            yml_config = yaml.load(f.read())
        default_config = {
            "port": 9292,
            "worker_num": 1,
            "build_dag_each_worker": False,
        }

        for key, val in default_config.items():
            if yml_config.get(key) is None:
                _LOGGER.warning("[CONF] {} not set, use default: {}"
                        .format(key, val))
                yml_config[key] = val

        self._port = yml_config["port"]
        if not self._port_is_available(self._port):
            raise SystemExit("Prot {} is already used".format(self._port))
        self._worker_num = yml_config["worker_num"]
        self._build_dag_each_worker = yml_config["build_dag_each_worker"]
        
        _LOGGER.info("============= PIPELINE SERVER =============")
        for key in default_config.keys():
            _LOGGER.info("{}: {}".format(key, yml_config[key]))
        if self._build_dag_each_worker is True:
            _LOGGER.info("(Make sure that install grpcio whl with --no-binary flag)")
        _LOGGER.info("-------------------------------------------")

        self._dag_config = yml_config.get("dag", {})
        self._dag_config["build_dag_each_worker"] = self._build_dag_each_worker

    def run_server(self):
        if self._build_dag_each_worker:
            with _reserve_port(self._port) as port:
                bind_address = 'localhost:{}'.format(port)
                workers = []
                for i in range(self._worker_num):
                    show_info = (i == 0)
                    worker = multiprocessing.Process(
                        target=self._run_server_func,
                        args=(bind_address, self._response_op,
                              self._dag_config))
                    worker.start()
                    workers.append(worker)
                for worker in workers:
                    worker.join()
        else:
            server = grpc.server(
                futures.ThreadPoolExecutor(max_workers=self._worker_num))
            pipeline_service_pb2_grpc.add_PipelineServiceServicer_to_server(
                PipelineServicer(self._response_op, self._dag_config, True),
                server)
            server.add_insecure_port('[::]:{}'.format(self._port))
            server.start()
            server.wait_for_termination()

    def _run_server_func(self, bind_address, response_op, dag_config):
        options = (('grpc.so_reuseport', 1), )
        server = grpc.server(
            futures.ThreadPoolExecutor(
                max_workers=1, ), options=options)
        pipeline_service_pb2_grpc.add_PipelineServiceServicer_to_server(
            PipelineServicer(response_op, dag_config, False), server)
        server.add_insecure_port(bind_address)
        server.start()
        server.wait_for_termination()
