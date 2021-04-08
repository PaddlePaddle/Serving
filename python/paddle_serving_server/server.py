# Copyright (c) 2021 PaddlePaddle Authors. All Rights Reserved.
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
import tarfile
import socket
import paddle_serving_server as paddle_serving_server
from paddle_serving_server.rpc_service import MultiLangServerServiceServicer
from .proto import server_configure_pb2 as server_sdk
from .proto import general_model_config_pb2 as m_config
from .proto import multi_lang_general_model_service_pb2_grpc
import google.protobuf.text_format
import time
from .version import serving_server_version, version_suffix, device_type
from contextlib import closing
import argparse

import sys
if sys.platform.startswith('win') is False:
    import fcntl
import shutil
import platform
import numpy as np
import grpc
import sys

from multiprocessing import Pool, Process
from concurrent import futures


class Server(object):
    def __init__(self):
        self.server_handle_ = None
        self.infer_service_conf = None
        self.model_toolkit_conf = None
        self.resource_conf = None
        self.memory_optimization = False
        self.ir_optimization = False
        self.model_conf = None
        self.workflow_fn = "workflow.prototxt"
        self.resource_fn = "resource.prototxt"
        self.infer_service_fn = "infer_service.prototxt"
        self.model_toolkit_fn = "model_toolkit.prototxt"
        self.general_model_config_fn = "general_model.prototxt"
        self.cube_config_fn = "cube.conf"
        self.workdir = ""
        self.max_concurrency = 0
        self.num_threads = 2
        self.port = 8080
        self.reload_interval_s = 10
        self.max_body_size = 64 * 1024 * 1024
        self.module_path = os.path.dirname(paddle_serving_server.__file__)
        self.cur_path = os.getcwd()
        self.use_local_bin = False
        self.mkl_flag = False
        self.device = "cpu"
        self.gpuid = 0
        self.use_trt = False
        self.use_lite = False
        self.use_xpu = False
        self.model_config_paths = None  # for multi-model in a workflow
        self.product_name = None
        self.container_id = None

    def get_fetch_list(self):
        fetch_names = [var.alias_name for var in self.model_conf.fetch_var]
        return fetch_names

    def set_max_concurrency(self, concurrency):
        self.max_concurrency = concurrency

    def set_num_threads(self, threads):
        self.num_threads = threads

    def set_max_body_size(self, body_size):
        if body_size >= self.max_body_size:
            self.max_body_size = body_size
        else:
            print(
                "max_body_size is less than default value, will use default value in service."
            )

    def use_encryption_model(self, flag=False):
        self.encryption_model = flag

    def set_port(self, port):
        self.port = port

    def set_reload_interval(self, interval):
        self.reload_interval_s = interval

    def set_op_sequence(self, op_seq):
        self.workflow_conf = op_seq

    def set_op_graph(self, op_graph):
        self.workflow_conf = op_graph

    def set_memory_optimize(self, flag=False):
        self.memory_optimization = flag

    def set_ir_optimize(self, flag=False):
        self.ir_optimization = flag

    def set_product_name(self, product_name=None):
        if product_name == None:
            raise ValueError("product_name can't be None.")
        self.product_name = product_name

    def set_container_id(self, container_id):
        if container_id == None:
            raise ValueError("container_id can't be None.")
        self.container_id = container_id

    def check_local_bin(self):
        if "SERVING_BIN" in os.environ:
            self.use_local_bin = True
            self.bin_path = os.environ["SERVING_BIN"]

    def check_cuda(self):
        if os.system("ls /dev/ | grep nvidia > /dev/null") == 0:
            pass
        else:
            raise SystemExit(
                "GPU not found, please check your environment or use cpu version by \"pip install paddle_serving_server\""
            )

    def set_device(self, device="cpu"):
        self.device = device

    def set_gpuid(self, gpuid=0):
        self.gpuid = gpuid

    def set_trt(self):
        self.use_trt = True

    def set_lite(self):
        self.use_lite = True

    def set_xpu(self):
        self.use_xpu = True

    def _prepare_engine(self, model_config_paths, device, use_encryption_model):
        if self.model_toolkit_conf == None:
            self.model_toolkit_conf = server_sdk.ModelToolkitConf()

        for engine_name, model_config_path in model_config_paths.items():
            engine = server_sdk.EngineDesc()
            engine.name = engine_name
            # engine.reloadable_meta = model_config_path + "/fluid_time_file"
            engine.reloadable_meta = self.workdir + "/fluid_time_file"
            os.system("touch {}".format(engine.reloadable_meta))
            engine.reloadable_type = "timestamp_ne"
            engine.runtime_thread_num = 0
            engine.batch_infer_size = 0
            engine.enable_batch_align = 0
            engine.model_dir = model_config_path
            engine.enable_memory_optimization = self.memory_optimization
            engine.enable_ir_optimization = self.ir_optimization
            engine.use_trt = self.use_trt
            engine.use_lite = self.use_lite
            engine.use_xpu = self.use_xpu
            if os.path.exists('{}/__params__'.format(model_config_path)):
                engine.combined_model = True
            else:
                engine.combined_model = False
            if use_encryption_model:
                engine.encrypted_model = True
            engine.type = "PADDLE_INFER"

            self.model_toolkit_conf.engines.extend([engine])

    def _prepare_infer_service(self, port):
        if self.infer_service_conf == None:
            self.infer_service_conf = server_sdk.InferServiceConf()
            self.infer_service_conf.port = port
            infer_service = server_sdk.InferService()
            infer_service.name = "GeneralModelService"
            infer_service.workflows.extend(["workflow1"])
            self.infer_service_conf.services.extend([infer_service])

    def _prepare_resource(self, workdir, cube_conf):
        self.workdir = workdir
        if self.resource_conf == None:
            with open("{}/{}".format(workdir, self.general_model_config_fn),
                      "w") as fout:
                fout.write(str(self.model_conf))
            self.resource_conf = server_sdk.ResourceConf()
            for workflow in self.workflow_conf.workflows:
                for node in workflow.nodes:
                    if "dist_kv" in node.name:
                        self.resource_conf.cube_config_path = workdir
                        self.resource_conf.cube_config_file = self.cube_config_fn
                        if cube_conf == None:
                            raise ValueError(
                                "Please set the path of cube.conf while use dist_kv op."
                            )
                        shutil.copy(cube_conf, workdir)
            self.resource_conf.model_toolkit_path = workdir
            self.resource_conf.model_toolkit_file = self.model_toolkit_fn
            self.resource_conf.general_model_path = workdir
            self.resource_conf.general_model_file = self.general_model_config_fn
            if self.product_name != None:
                self.resource_conf.auth_product_name = self.product_name
            if self.container_id != None:
                self.resource_conf.auth_container_id = self.container_id

    def _write_pb_str(self, filepath, pb_obj):
        with open(filepath, "w") as fout:
            fout.write(str(pb_obj))

    def load_model_config(self, model_config_paths):
        # At present, Serving needs to configure the model path in
        # the resource.prototxt file to determine the input and output
        # format of the workflow. To ensure that the input and output
        # of multiple models are the same.
        workflow_oi_config_path = None
        if isinstance(model_config_paths, str):
            # If there is only one model path, use the default infer_op.
            # Because there are several infer_op type, we need to find
            # it from workflow_conf.
            default_engine_names = [
                'general_infer_0', 'general_dist_kv_infer_0',
                'general_dist_kv_quant_infer_0'
            ]
            engine_name = None
            for node in self.workflow_conf.workflows[0].nodes:
                if node.name in default_engine_names:
                    engine_name = node.name
                    break
            if engine_name is None:
                raise Exception(
                    "You have set the engine_name of Op. Please use the form {op: model_path} to configure model path"
                )
            self.model_config_paths = {engine_name: model_config_paths}
            workflow_oi_config_path = self.model_config_paths[engine_name]
        elif isinstance(model_config_paths, dict):
            self.model_config_paths = {}
            for node_str, path in model_config_paths.items():
                node = server_sdk.DAGNode()
                google.protobuf.text_format.Parse(node_str, node)
                self.model_config_paths[node.name] = path
            print("You have specified multiple model paths, please ensure "
                  "that the input and output of multiple models are the same.")
            workflow_oi_config_path = list(self.model_config_paths.items())[0][
                1]
        else:
            raise Exception("The type of model_config_paths must be str or "
                            "dict({op: model_path}), not {}.".format(
                                type(model_config_paths)))

        self.model_conf = m_config.GeneralModelConfig()
        f = open(
            "{}/serving_server_conf.prototxt".format(workflow_oi_config_path),
            'r')
        self.model_conf = google.protobuf.text_format.Merge(
            str(f.read()), self.model_conf)
        # check config here
        # print config here

    def use_mkl(self, flag):
        self.mkl_flag = flag

    def get_device_version(self):
        avx_flag = False
        mkl_flag = self.mkl_flag
        openblas_flag = False
        r = os.system("cat /proc/cpuinfo | grep avx > /dev/null 2>&1")
        if r == 0:
            avx_flag = True
        if avx_flag:
            if mkl_flag:
                device_version = "cpu-avx-mkl"
            else:
                device_version = "cpu-avx-openblas"
        else:
            if mkl_flag:
                print(
                    "Your CPU does not support AVX, server will running with noavx-openblas mode."
                )
            device_version = "cpu-noavx-openblas"
        return device_version

    def get_serving_bin_name(self):
        if device_type == "0":
            device_version = self.get_device_version()
        elif device_type == "1":
            if version_suffix == "101" or version_suffix == "102":
                device_version = "gpu-" + version_suffix
            else:
                device_version = "gpu-cuda" + version_suffix
        elif device_type == "2":
            device_version = "xpu-" + platform.machine()
        return device_version

    def download_bin(self):
        os.chdir(self.module_path)
        need_download = False

        #acquire lock
        version_file = open("{}/version.py".format(self.module_path), "r")

        folder_name = "serving-%s-%s" % (self.get_serving_bin_name(),
                                         serving_server_version)
        tar_name = "%s.tar.gz" % folder_name
        bin_url = "https://paddle-serving.bj.bcebos.com/bin/%s" % tar_name

        self.server_path = os.path.join(self.module_path, folder_name)

        download_flag = "{}/{}.is_download".format(self.module_path,
                                                   folder_name)

        fcntl.flock(version_file, fcntl.LOCK_EX)

        if os.path.exists(download_flag):
            os.chdir(self.cur_path)
            self.bin_path = self.server_path + "/serving"
            return

        if not os.path.exists(self.server_path):
            os.system("touch {}/{}.is_download".format(self.module_path,
                                                       folder_name))
            print('Frist time run, downloading PaddleServing components ...')

            r = os.system('wget ' + bin_url + ' --no-check-certificate')
            if r != 0:
                if os.path.exists(tar_name):
                    os.remove(tar_name)
                raise SystemExit(
                    'Download failed, please check your network or permission of {}.'
                    .format(self.module_path))
            else:
                try:
                    print('Decompressing files ..')
                    tar = tarfile.open(tar_name)
                    tar.extractall()
                    tar.close()
                except:
                    if os.path.exists(exe_path):
                        os.remove(exe_path)
                    raise SystemExit(
                        'Decompressing failed, please check your permission of {} or disk space left.'
                        .format(self.module_path))
                finally:
                    os.remove(tar_name)
        #release lock
        version_file.close()
        os.chdir(self.cur_path)
        self.bin_path = self.server_path + "/serving"

    def prepare_server(self,
                       workdir=None,
                       port=9292,
                       device="cpu",
                       use_encryption_model=False,
                       cube_conf=None):
        if workdir == None:
            workdir = "./tmp"
            os.system("mkdir -p {}".format(workdir))
        else:
            os.system("mkdir -p {}".format(workdir))
        os.system("touch {}/fluid_time_file".format(workdir))

        if not self.port_is_available(port):
            raise SystemExit("Port {} is already used".format(port))

        self.set_port(port)
        self._prepare_resource(workdir, cube_conf)
        self._prepare_engine(self.model_config_paths, device,
                             use_encryption_model)
        self._prepare_infer_service(port)
        self.workdir = workdir

        infer_service_fn = "{}/{}".format(workdir, self.infer_service_fn)
        workflow_fn = "{}/{}".format(workdir, self.workflow_fn)
        resource_fn = "{}/{}".format(workdir, self.resource_fn)
        model_toolkit_fn = "{}/{}".format(workdir, self.model_toolkit_fn)

        self._write_pb_str(infer_service_fn, self.infer_service_conf)
        self._write_pb_str(workflow_fn, self.workflow_conf)
        self._write_pb_str(resource_fn, self.resource_conf)
        self._write_pb_str(model_toolkit_fn, self.model_toolkit_conf)

    def port_is_available(self, port):
        with closing(socket.socket(socket.AF_INET, socket.SOCK_STREAM)) as sock:
            sock.settimeout(2)
            result = sock.connect_ex(('0.0.0.0', port))
        if result != 0:
            return True
        else:
            return False

    def run_server(self):
        # just run server with system command
        # currently we do not load cube
        self.check_local_bin()
        if not self.use_local_bin:
            self.download_bin()
            # wait for other process to download server bin
            while not os.path.exists(self.server_path):
                time.sleep(1)
        else:
            print("Use local bin : {}".format(self.bin_path))
        #self.check_cuda()
        # Todo: merge CPU and GPU code, remove device to model_toolkit
        if self.device == "cpu" or self.device == "arm":
            command = "{} " \
                      "-enable_model_toolkit " \
                      "-inferservice_path {} " \
                      "-inferservice_file {} " \
                      "-max_concurrency {} " \
                      "-num_threads {} " \
                      "-port {} " \
                      "-reload_interval_s {} " \
                      "-resource_path {} " \
                      "-resource_file {} " \
                      "-workflow_path {} " \
                      "-workflow_file {} " \
                      "-bthread_concurrency {} " \
                      "-max_body_size {} ".format(
                          self.bin_path,
                          self.workdir,
                          self.infer_service_fn,
                          self.max_concurrency,
                          self.num_threads,
                          self.port,
                          self.reload_interval_s,
                          self.workdir,
                          self.resource_fn,
                          self.workdir,
                          self.workflow_fn,
                          self.num_threads,
                          self.max_body_size)
        else:
            command = "{} " \
                      "-enable_model_toolkit " \
                      "-inferservice_path {} " \
                      "-inferservice_file {} " \
                      "-max_concurrency {} " \
                      "-num_threads {} " \
                      "-port {} " \
                      "-reload_interval_s {} " \
                      "-resource_path {} " \
                      "-resource_file {} " \
                      "-workflow_path {} " \
                      "-workflow_file {} " \
                      "-bthread_concurrency {} " \
                      "-gpuid {} " \
                      "-max_body_size {} ".format(
                          self.bin_path,
                          self.workdir,
                          self.infer_service_fn,
                          self.max_concurrency,
                          self.num_threads,
                          self.port,
                          self.reload_interval_s,
                          self.workdir,
                          self.resource_fn,
                          self.workdir,
                          self.workflow_fn,
                          self.num_threads,
                          self.gpuid,
                          self.max_body_size)
        print("Going to Run Comand")
        print(command)

        os.system(command)


class MultiLangServer(object):
    def __init__(self):
        self.bserver_ = Server()
        self.worker_num_ = 4
        self.body_size_ = 64 * 1024 * 1024
        self.concurrency_ = 100000
        self.is_multi_model_ = False  # for model ensemble

    def set_max_concurrency(self, concurrency):
        self.concurrency_ = concurrency
        self.bserver_.set_max_concurrency(concurrency)

    def set_device(self, device="cpu"):
        self.device = device

    def set_num_threads(self, threads):
        self.worker_num_ = threads
        self.bserver_.set_num_threads(threads)

    def set_max_body_size(self, body_size):
        self.bserver_.set_max_body_size(body_size)
        if body_size >= self.body_size_:
            self.body_size_ = body_size
        else:
            print(
                "max_body_size is less than default value, will use default value in service."
            )

    def use_encryption_model(self, flag=False):
        self.encryption_model = flag

    def set_port(self, port):
        self.gport_ = port

    def set_reload_interval(self, interval):
        self.bserver_.set_reload_interval(interval)

    def set_op_sequence(self, op_seq):
        self.bserver_.set_op_sequence(op_seq)

    def set_op_graph(self, op_graph):
        self.bserver_.set_op_graph(op_graph)

    def use_mkl(self, flag):
        self.bserver_.use_mkl(flag)

    def set_memory_optimize(self, flag=False):
        self.bserver_.set_memory_optimize(flag)

    def set_ir_optimize(self, flag=False):
        self.bserver_.set_ir_optimize(flag)

    def set_gpuid(self, gpuid=0):
        self.bserver_.set_gpuid(gpuid)

    def load_model_config(self, server_config_paths, client_config_path=None):
        self.bserver_.load_model_config(server_config_paths)
        if client_config_path is None:
            if isinstance(server_config_paths, dict):
                self.is_multi_model_ = True
                client_config_path = '{}/serving_server_conf.prototxt'.format(
                    list(server_config_paths.items())[0][1])
            else:
                client_config_path = '{}/serving_server_conf.prototxt'.format(
                    server_config_paths)
        self.bclient_config_path_ = client_config_path

    def prepare_server(self,
                       workdir=None,
                       port=9292,
                       device="cpu",
                       use_encryption_model=False,
                       cube_conf=None):
        if not self._port_is_available(port):
            raise SystemExit("Prot {} is already used".format(port))
        default_port = 12000
        self.port_list_ = []
        for i in range(1000):
            if default_port + i != port and self._port_is_available(default_port
                                                                    + i):
                self.port_list_.append(default_port + i)
                break
        self.bserver_.prepare_server(
            workdir=workdir,
            port=self.port_list_[0],
            device=device,
            use_encryption_model=use_encryption_model,
            cube_conf=cube_conf)
        self.set_port(port)

    def _launch_brpc_service(self, bserver):
        bserver.run_server()

    def _port_is_available(self, port):
        with closing(socket.socket(socket.AF_INET, socket.SOCK_STREAM)) as sock:
            sock.settimeout(2)
            result = sock.connect_ex(('0.0.0.0', port))
        return result != 0

    def run_server(self):
        p_bserver = Process(
            target=self._launch_brpc_service, args=(self.bserver_, ))
        p_bserver.start()
        options = [('grpc.max_send_message_length', self.body_size_),
                   ('grpc.max_receive_message_length', self.body_size_)]
        server = grpc.server(
            futures.ThreadPoolExecutor(max_workers=self.worker_num_),
            options=options,
            maximum_concurrent_rpcs=self.concurrency_)
        multi_lang_general_model_service_pb2_grpc.add_MultiLangGeneralModelServiceServicer_to_server(
            MultiLangServerServiceServicer(
                self.bclient_config_path_, self.is_multi_model_,
                ["0.0.0.0:{}".format(self.port_list_[0])]), server)
        server.add_insecure_port('[::]:{}'.format(self.gport_))
        server.start()
        p_bserver.join()
        server.wait_for_termination()
