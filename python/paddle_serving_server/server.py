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
from paddle_serving_server.serve import format_gpu_to_strlist
from .proto import server_configure_pb2 as server_sdk
from .proto import general_model_config_pb2 as m_config
import google.protobuf.text_format
import time
from .version import version_tag, version_suffix, device_type
from contextlib import closing
import argparse

import sys
if sys.platform.startswith('win') is False:
    import fcntl
import shutil
import platform
import numpy as np
import sys
import collections
import subprocess

from multiprocessing import Pool, Process
from concurrent import futures


# The whole file is about to be discarded.
# We will use default config-file to start C++Server.
class Server(object):
    def __init__(self):
        """
        self.model_toolkit_conf:'list'=[] # The quantity of self.model_toolkit_conf is equal to the InferOp quantity/Engine--OP
        self.model_conf:'collections.OrderedDict()' # Save the serving_server_conf.prototxt content (feed and fetch information) this is a map for multi-model in a workflow
        self.workflow_fn:'str'="workflow.prototxt" # Only one for one Service/Workflow
        self.resource_fn:'str'="resource.prototxt" # Only one for one Service,model_toolkit_fn and general_model_config_fn is recorded in this file
        self.infer_service_fn:'str'="infer_service.prototxt" # Only one for one Service,Service--Workflow
        self.model_toolkit_fn:'list'=[] # ["GeneralInferOp_0/model_toolkit.prototxt"]The quantity is equal to the InferOp quantity,Engine--OP
        self.general_model_config_fn:'list'=[] # ["GeneralInferOp_0/general_model.prototxt"]The quantity is equal to the InferOp quantity,Feed and Fetch --OP
        self.subdirectory:'list'=[] # The quantity is equal to the InferOp quantity, and name = node.name = engine.name
        self.model_config_paths:'collections.OrderedDict()' # Save the serving_server_conf.prototxt path (feed and fetch information) this is a map for multi-model in a workflow
        """
        self.server_handle_ = None
        self.infer_service_conf = None
        self.model_toolkit_conf = []
        self.resource_conf = None
        self.memory_optimization = False
        self.ir_optimization = False
        self.model_conf = collections.OrderedDict()
        self.workflow_fn = "workflow.prototxt"
        self.resource_fn = "resource.prototxt"
        self.infer_service_fn = "infer_service.prototxt"
        self.model_toolkit_fn = []
        self.general_model_config_fn = []
        self.subdirectory = []
        self.cube_config_fn = "cube.conf"
        self.workdir = ""
        self.max_concurrency = 0
        self.num_threads = 2
        self.port = 8080
        self.precision = "fp32"
        self.use_calib = False
        self.reload_interval_s = 10
        self.max_body_size = 64 * 1024 * 1024
        self.module_path = os.path.dirname(paddle_serving_server.__file__)
        self.cur_path = os.getcwd()
        self.use_local_bin = False
        self.mkl_flag = False
        self.device = "cpu"
        self.gpuid = []
        self.runtime_thread_num = [0]
        self.batch_infer_size = [32]
        self.use_trt = False
        self.gpu_multi_stream = False
        self.use_lite = False
        self.use_xpu = False
        self.use_ascend_cl = False
        self.model_config_paths = collections.OrderedDict()
        self.product_name = None
        self.container_id = None
        self.default_engine_types = [
            'GeneralInferOp',
            'GeneralDistKVInferOp',
            'GeneralDistKVQuantInferOp',
            'GeneralDetectionOp',
        ]
        self.enable_prometheus = False
        self.prometheus_port = 19393
        self.request_cache_size = 0

    def get_fetch_list(self, infer_node_idx=-1):
        fetch_names = [
            var.alias_name
            for var in list(self.model_conf.values())[infer_node_idx].fetch_var
        ]
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

    def set_precision(self, precision="fp32"):
        self.precision = precision

    def set_use_calib(self, use_calib=False):
        self.use_calib = use_calib

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

    # Multi-Server does not have this Function.
    def set_product_name(self, product_name=None):
        if product_name == None:
            raise ValueError("product_name can't be None.")
        self.product_name = product_name

    # Multi-Server does not have this Function.
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

    def set_gpuid(self, gpuid):
        self.gpuid = format_gpu_to_strlist(gpuid)

    def set_runtime_thread_num(self, runtime_thread_num):
        self.runtime_thread_num = runtime_thread_num

    def set_batch_infer_size(self, batch_infer_size):
        self.batch_infer_size = batch_infer_size

    def set_trt(self):
        self.use_trt = True

    def set_gpu_multi_stream(self):
        self.gpu_multi_stream = True

    def set_lite(self):
        self.use_lite = True

    def set_xpu(self):
        self.use_xpu = True

    def set_ascend_cl(self):
        self.use_ascend_cl = True

    def set_enable_prometheus(self, flag=False):
        self.enable_prometheus = flag

    def set_prometheus_port(self, prometheus_port):
        self.prometheus_port = prometheus_port

    def set_request_cache_size(self, request_cache_size):
        self.request_cache_size = request_cache_size

    def _prepare_engine(self, model_config_paths, device, use_encryption_model):
        self.device = device
        if self.model_toolkit_conf == None:
            self.model_toolkit_conf = []

        # Generally, self.gpuid = str[] or [].
        # when len(self.gpuid) means no gpuid is specified.
        # if self.device == "gpu" or self.use_trt:
        # we assume you forget to set gpuid, so set gpuid = ['0'];
        if len(self.gpuid) == 0 or self.gpuid == ["-1"]:
            if self.device == "gpu" or self.use_trt or self.gpu_multi_stream:
                self.gpuid = ["0"]
                self.device = "gpu"
            elif self.use_xpu or self.use_ascend_cl:
                self.gpuid = ["0"]
            else:
                self.gpuid = ["-1"]

        if isinstance(self.runtime_thread_num, int):
            self.runtime_thread_num = [self.runtime_thread_num]
        if len(self.runtime_thread_num) == 0:
            self.runtime_thread_num.append(0)

        if isinstance(self.batch_infer_size, int):
            self.batch_infer_size = [self.batch_infer_size]
        if len(self.batch_infer_size) == 0:
            self.batch_infer_size.append(32)

        index = 0

        for engine_name, model_config_path in model_config_paths.items():
            engine = server_sdk.EngineDesc()
            engine.name = engine_name
            # engine.reloadable_meta = model_config_path + "/fluid_time_file"
            engine.reloadable_meta = model_config_path + "/fluid_time_file"
            os.system("touch {}".format(engine.reloadable_meta))
            engine.reloadable_type = "timestamp_ne"
            engine.runtime_thread_num = self.runtime_thread_num[index % len(
                self.runtime_thread_num)]
            engine.batch_infer_size = self.batch_infer_size[index % len(
                self.batch_infer_size)]

            engine.enable_overrun = False
            engine.allow_split_request = True
            engine.model_dir = model_config_path
            engine.enable_memory_optimization = self.memory_optimization
            engine.enable_ir_optimization = self.ir_optimization
            engine.use_trt = self.use_trt
            engine.gpu_multi_stream = self.gpu_multi_stream
            engine.use_lite = self.use_lite
            engine.use_xpu = self.use_xpu
            engine.use_ascend_cl = self.use_ascend_cl
            engine.use_gpu = False

            if len(self.gpuid) == 0:
                raise ValueError("CPU: self.gpuid = -1, GPU: must set it ")
            op_gpu_list = self.gpuid[index % len(self.gpuid)].split(",")
            for ids in op_gpu_list:
                engine.gpu_ids.extend([int(ids)])

            if self.device == "gpu" or self.use_trt or self.gpu_multi_stream:
                engine.use_gpu = True
                # this is for Mixed use of GPU and CPU
                # if model-1 use GPU and set the device="gpu"
                # but gpuid[1] = "-1" which means use CPU in Model-2
                # so config about GPU should be False.
                # op_gpu_list = gpuid[index].split(",")
                # which is the gpuid for each engine.
                if len(op_gpu_list) == 1:
                    if int(op_gpu_list[0]) == -1:
                        engine.use_gpu = False
                        engine.gpu_multi_stream = False
                        engine.use_trt = False

            if os.path.exists('{}/__params__'.format(model_config_path)):
                engine.combined_model = True
            else:
                engine.combined_model = False
            if use_encryption_model:
                engine.encrypted_model = True
            engine.type = "PADDLE_INFER"
            self.model_toolkit_conf.append(server_sdk.ModelToolkitConf())
            self.model_toolkit_conf[-1].engines.extend([engine])
            index = index + 1

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
            self.resource_conf = server_sdk.ResourceConf()
            for idx, op_general_model_config_fn in enumerate(
                    self.general_model_config_fn):
                with open("{}/{}".format(workdir, op_general_model_config_fn),
                          "w") as fout:
                    fout.write(str(list(self.model_conf.values())[idx]))
                for workflow in self.workflow_conf.workflows:
                    for node in workflow.nodes:
                        if "distkv" in node.name.lower():
                            self.resource_conf.cube_config_path = workdir
                            self.resource_conf.cube_config_file = self.cube_config_fn
                            if cube_conf == None:
                                raise ValueError(
                                    "Please set the path of cube.conf while use dist_kv op."
                                )
                            shutil.copy(cube_conf, workdir)
                            if "quant" in node.name.lower():
                                self.resource_conf.cube_quant_bits = 8
                self.resource_conf.model_toolkit_path.extend([workdir])
                self.resource_conf.model_toolkit_file.extend(
                    [self.model_toolkit_fn[idx]])
                self.resource_conf.general_model_path.extend([workdir])
                self.resource_conf.general_model_file.extend(
                    [op_general_model_config_fn])
                #TODO:figure out the meaning of product_name and container_id.
                if self.product_name != None:
                    self.resource_conf.auth_product_name = self.product_name
                if self.container_id != None:
                    self.resource_conf.auth_container_id = self.container_id

    def _write_pb_str(self, filepath, pb_obj):
        with open(filepath, "w") as fout:
            fout.write(str(pb_obj))

    def load_model_config(self, model_config_paths_args):
        # At present, Serving needs to configure the model path in
        # the resource.prototxt file to determine the input and output
        # format of the workflow. To ensure that the input and output
        # of multiple models are the same.
        if isinstance(model_config_paths_args, str):
            model_config_paths_args = [model_config_paths_args]

        for single_model_config in model_config_paths_args:
            if os.path.isdir(single_model_config):
                pass
            elif os.path.isfile(single_model_config):
                raise ValueError(
                    "The input of --model should be a dir not file.")

        if isinstance(model_config_paths_args, list):
            # If there is only one model path, use the default infer_op.
            # Because there are several infer_op type, we need to find
            # it from workflow_conf.

            # now only support single-workflow.
            # TODO:support multi-workflow
            model_config_paths_list_idx = 0
            for node in self.workflow_conf.workflows[0].nodes:
                if node.type in self.default_engine_types:
                    if node.name is None:
                        raise Exception(
                            "You have set the engine_name of Op. Please use the form {op: model_path} to configure model path"
                        )

                    f = open("{}/serving_server_conf.prototxt".format(
                        model_config_paths_args[model_config_paths_list_idx]),
                             'r')
                    self.model_conf[
                        node.name] = google.protobuf.text_format.Merge(
                            str(f.read()), m_config.GeneralModelConfig())
                    self.model_config_paths[
                        node.name] = model_config_paths_args[
                            model_config_paths_list_idx]
                    self.general_model_config_fn.append(
                        node.name + "/general_model.prototxt")
                    self.model_toolkit_fn.append(node.name +
                                                 "/model_toolkit.prototxt")
                    self.subdirectory.append(node.name)
                    model_config_paths_list_idx += 1
                    if model_config_paths_list_idx == len(
                            model_config_paths_args):
                        break
        #Right now, this is not useful.
        elif isinstance(model_config_paths_args, dict):
            self.model_config_paths = collections.OrderedDict()
            for node_str, path in model_config_paths_args.items():
                node = server_sdk.DAGNode()
                google.protobuf.text_format.Parse(node_str, node)
                self.model_config_paths[node.name] = path
            print("You have specified multiple model paths, please ensure "
                  "that the input and output of multiple models are the same.")
            f = open("{}/serving_server_conf.prototxt".format(path), 'r')
            self.model_conf[node.name] = google.protobuf.text_format.Merge(
                str(f.read()), m_config.GeneralModelConfig())
        else:
            raise Exception(
                "The type of model_config_paths must be str or list or "
                "dict({op: model_path}), not {}.".format(
                    type(model_config_paths_args)))
        # check config here
        # print config here

    def use_mkl(self, flag):
        self.mkl_flag = flag

    def check_avx(self):
        p = subprocess.Popen(
            ['cat /proc/cpuinfo | grep avx 2>/dev/null'],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            shell=True)
        out, err = p.communicate()
        if err == b'' and len(out) > 0:
            return True
        else:
            return False

    def get_device_version(self):
        avx_flag = False
        avx_support = self.check_avx()
        if avx_support:
            avx_flag = True
            self.use_mkl(True)
        mkl_flag = self.mkl_flag
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
            if version_suffix == "101" or version_suffix == "102" or version_suffix == "1028" or version_suffix == "112":
                device_version = "gpu-" + version_suffix
            else:
                device_version = "gpu-cuda" + version_suffix
        elif device_type == "2":
            device_version = "xpu-" + platform.machine()
        elif device_type == "3":
            device_version = "rocm-" + platform.machine()
        elif device_type == "4":
            if self.use_lite:
                device_version = "ascendcl-lite-" + platform.machine()
            else:
                device_version = "ascendcl-" + platform.machine()
        return device_version

    def download_bin(self):
        os.chdir(self.module_path)

        #acquire lock
        version_file = open("{}/version.py".format(self.module_path), "r")

        folder_name = "serving-%s-%s" % (self.get_serving_bin_name(),
                                         version_tag)
        tar_name = "%s.tar.gz" % folder_name
        bin_url = "https://paddle-serving.bj.bcebos.com/test-dev/bin/%s" % tar_name

        self.server_path = os.path.join(self.module_path, folder_name)

        download_flag = "{}/{}.is_download".format(self.module_path,
                                                   folder_name)

        fcntl.flock(version_file, fcntl.LOCK_EX)

        if os.path.exists(download_flag):
            os.chdir(self.cur_path)
            self.bin_path = self.server_path + "/serving"
            return

        if not os.path.exists(self.server_path):
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
                    open(download_flag, "a").close()
                except:
                    if os.path.exists(self.server_path):
                        os.remove(self.server_path)
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
                       device=None,
                       use_encryption_model=False,
                       cube_conf=None):
        # if `device` is not set, use self.device
        # self.device may not be changed.
        # or self.device may have changed by set_device.
        if device == None:
            device = self.device
        # if `device` is set, let self.device = device.
        else:
            self.device = device
        if workdir == None:
            workdir = "./tmp"
            os.system("mkdir -p {}".format(workdir))
        else:
            os.system("mkdir -p {}".format(workdir))
        for subdir in self.subdirectory:
            os.system("mkdir -p {}/{}".format(workdir, subdir))
            os.system("touch {}/{}/fluid_time_file".format(workdir, subdir))

        if not self.port_is_available(port):
            raise SystemExit("Port {} is already used".format(port))

        self.set_port(port)
        self._prepare_resource(workdir, cube_conf)
        self._prepare_engine(self.model_config_paths, device,
                             use_encryption_model)
        self._prepare_infer_service(port)
        self.workdir = workdir

        infer_service_fn = "{}/{}".format(workdir, self.infer_service_fn)
        self._write_pb_str(infer_service_fn, self.infer_service_conf)

        workflow_fn = "{}/{}".format(workdir, self.workflow_fn)
        self._write_pb_str(workflow_fn, self.workflow_conf)

        resource_fn = "{}/{}".format(workdir, self.resource_fn)
        self._write_pb_str(resource_fn, self.resource_conf)

        for idx, single_model_toolkit_fn in enumerate(self.model_toolkit_fn):
            model_toolkit_fn = "{}/{}".format(workdir, single_model_toolkit_fn)
            self._write_pb_str(model_toolkit_fn, self.model_toolkit_conf[idx])

    def port_is_available(self, port):
        with closing(socket.socket(socket.AF_INET, socket.SOCK_STREAM)) as sock:
            sock.settimeout(2)
            result = sock.connect_ex(('127.0.0.1', port))
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
        command = "{} " \
                    "-enable_model_toolkit " \
                    "-inferservice_path {} " \
                    "-inferservice_file {} " \
                    "-max_concurrency {} " \
                    "-num_threads {} " \
                    "-port {} " \
                    "-precision {} " \
                    "-use_calib={} " \
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
                        self.precision,
                        self.use_calib,
                        self.reload_interval_s,
                        self.workdir,
                        self.resource_fn,
                        self.workdir,
                        self.workflow_fn,
                        self.num_threads,
                        self.max_body_size)
        if self.enable_prometheus:
            command =   command + \
                        "-enable_prometheus={} " \
                        "-prometheus_port {} ".format(
                        self.enable_prometheus,
                        self.prometheus_port)
        if self.request_cache_size > 0:
            command =   command + \
                        "-request_cache_size {} ".format(
                            self.request_cache_size
                        )

        print("Going to Run Comand")
        print(command)

        os.system(command)
