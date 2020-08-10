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


class PipelineService(pipeline_service_pb2_grpc.PipelineServiceServicer):
    def __init__(self, response_op, dag_config, show_info):
        super(PipelineService, self).__init__()
        # init dag executor
        self._dag_executor = DAGExecutor(
            response_op, dag_config, show_info=show_info)
        self._dag_executor.start()

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
        self._port = yml_config.get('port')
        if self._port is None:
            raise SystemExit("Please set *port* in [{}] yaml file.".format(
                yml_file))
        if not self._port_is_available(self._port):
            raise SystemExit("Prot {} is already used".format(self._port))
        self._worker_num = yml_config.get('worker_num', 1)
        self._build_dag_each_worker = yml_config.get('build_dag_each_worker',
                                                     False)
        _LOGGER.info("============= PIPELINE SERVER =============")
        _LOGGER.info("port: {}".format(self._port))
        _LOGGER.info("worker_num: {}".format(self._worker_num))
        servicer_info = "build_dag_each_worker: {}".format(
            self._build_dag_each_worker)
        if self._build_dag_each_worker is True:
            servicer_info += " (Make sure that install grpcio whl with --no-binary flag)"
        _LOGGER.info(servicer_info)
        _LOGGER.info("-------------------------------------------")

        self._dag_config = yml_config.get("dag", {})

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
                PipelineService(self._response_op, self._dag_config, True),
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
            PipelineService(response_op, dag_config, False), server)
        server.add_insecure_port(bind_address)
        server.start()
        server.wait_for_termination()
