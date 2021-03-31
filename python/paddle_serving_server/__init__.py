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

import os
from .proto import server_configure_pb2 as server_sdk
from .proto import general_model_config_pb2 as m_config
import google.protobuf.text_format
import tarfile
import socket
import paddle_serving_server as paddle_serving_server
from .version import serving_server_version
from contextlib import closing
import collections
import shutil
import numpy as np
import grpc
from .proto import multi_lang_general_model_service_pb2
import sys
if sys.platform.startswith('win') is False:
    import fcntl
sys.path.append(
    os.path.join(os.path.abspath(os.path.dirname(__file__)), 'proto'))
from .proto import multi_lang_general_model_service_pb2_grpc
from multiprocessing import Pool, Process
from concurrent import futures


class OpMaker(object):
    def __init__(self):
        self.op_dict = {
            "general_infer": "GeneralInferOp",
            "general_reader": "GeneralReaderOp",
            "general_response": "GeneralResponseOp",
            "general_text_reader": "GeneralTextReaderOp",
            "general_text_response": "GeneralTextResponseOp",
            "general_single_kv": "GeneralSingleKVOp",
            "general_dist_kv_infer": "GeneralDistKVInferOp",
            "general_dist_kv_quant_infer": "GeneralDistKVQuantInferOp",
            "general_copy": "GeneralCopyOp",
            "general_YSL":"GeneralYSLOp",
        }
        self.node_name_suffix_ = collections.defaultdict(int)

    def create(self, node_type, engine_name=None, inputs=[], outputs=[]):
        if node_type not in self.op_dict:
            raise Exception("Op type {} is not supported right now".format(
                node_type))
        node = server_sdk.DAGNode()
        # node.name will be used as the infer engine name
        if engine_name:
            node.name = engine_name
        else:
            node.name = '{}_{}'.format(node_type,
                                       self.node_name_suffix_[node_type])
            self.node_name_suffix_[node_type] += 1

        node.type = self.op_dict[node_type]
        if inputs:
            for dep_node_str in inputs:
                dep_node = server_sdk.DAGNode()
                google.protobuf.text_format.Parse(dep_node_str, dep_node)
                dep = server_sdk.DAGNodeDependency()
                dep.name = dep_node.name
                dep.mode = "RO"
                node.dependencies.extend([dep])
        # Because the return value will be used as the key value of the
        # dict, and the proto object is variable which cannot be hashed,
        # so it is processed into a string. This has little effect on
        # overall efficiency.
        return google.protobuf.text_format.MessageToString(node)


class OpSeqMaker(object):
    def __init__(self):
        self.workflow = server_sdk.Workflow()
        self.workflow.name = "workflow1"
        self.workflow.workflow_type = "Sequence"

    def add_op(self, node_str):
        node = server_sdk.DAGNode()
        google.protobuf.text_format.Parse(node_str, node)
        if len(node.dependencies) > 1:
            raise Exception(
                'Set more than one predecessor for op in OpSeqMaker is not allowed.'
            )
        if len(self.workflow.nodes) >= 1:
            if len(node.dependencies) == 0:
                dep = server_sdk.DAGNodeDependency()
                dep.name = self.workflow.nodes[-1].name
                dep.mode = "RO"
                node.dependencies.extend([dep])
            elif len(node.dependencies) == 1:
                if node.dependencies[0].name != self.workflow.nodes[-1].name:
                    raise Exception(
                        'You must add op in order in OpSeqMaker. The previous op is {}, but the current op is followed by {}.'
                        .format(node.dependencies[0].name, self.workflow.nodes[
                            -1].name))
        self.workflow.nodes.extend([node])

    def get_op_sequence(self):
        workflow_conf = server_sdk.WorkflowConf()
        workflow_conf.workflows.extend([self.workflow])
        return workflow_conf


class OpGraphMaker(object):
    def __init__(self):
        self.workflow = server_sdk.Workflow()
        self.workflow.name = "workflow1"
        # Currently, SDK only supports "Sequence"
        self.workflow.workflow_type = "Sequence"

    def add_op(self, node_str):
        node = server_sdk.DAGNode()
        google.protobuf.text_format.Parse(node_str, node)
        self.workflow.nodes.extend([node])

    def get_op_graph(self):
        workflow_conf = server_sdk.WorkflowConf()
        workflow_conf.workflows.extend([self.workflow])
        return workflow_conf


class Server(object):
    def __init__(self):
        self.server_handle_ = None
        self.infer_service_conf = None
        self.model_toolkit_conf = []#The quantity is equal to the InferOp quantity,Engine--OP
        self.resource_conf = None
        self.memory_optimization = False
        self.ir_optimization = False
        self.model_conf = collections.OrderedDict()# save the serving_server_conf.prototxt content (feed and fetch information) this is a map for multi-model in a workflow
        self.workflow_fn = "workflow.prototxt"#only one for one Service,Workflow--Op 
        self.resource_fn = "resource.prototxt"#only one for one Service,model_toolkit_fn and  general_model_config_fn is recorded in this file
        self.infer_service_fn = "infer_service.prototxt"#only one for one Service,Service--Workflow
        self.model_toolkit_fn = []#["general_infer_0/model_toolkit.prototxt"]The quantity is equal to the InferOp quantity,Engine--OP
        self.general_model_config_fn = []#["general_infer_0/general_model.prototxt"]The quantity is equal to the InferOp quantity,Feed and Fetch --OP
        self.subdirectory = []#The quantity is equal to the InferOp quantity, and name = node.name = engine.name
        self.cube_config_fn = "cube.conf"
        self.workdir = ""
        self.max_concurrency = 0
        self.num_threads = 4
        self.port = 8080
        self.reload_interval_s = 10
        self.max_body_size = 64 * 1024 * 1024
        self.module_path = os.path.dirname(paddle_serving_server.__file__)
        self.cur_path = os.getcwd()
        self.use_local_bin = False
        self.mkl_flag = False
        self.encryption_model = False
        self.product_name = None
        self.container_id = None
        self.model_config_paths = collections.OrderedDict()  # save the serving_server_conf.prototxt path (feed and fetch information) this is a map for multi-model in a workflow

    def get_fetch_list(self,infer_node_idx = -1 ):
        fetch_names = [var.alias_name for var in list(self.model_conf.values())[infer_node_idx].fetch_var]
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

    def use_encryption_model(self, flag=False):
        self.encryption_model = flag

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

    def _prepare_engine(self, model_config_paths, device):
        if self.model_toolkit_conf == None:
            self.model_toolkit_conf = []
        for engine_name, model_config_path in model_config_paths.items():
            engine = server_sdk.EngineDesc()
            engine.name = engine_name
            engine.reloadable_meta = model_config_path + "/fluid_time_file"
            os.system("touch {}".format(engine.reloadable_meta))
            engine.reloadable_type = "timestamp_ne"
            engine.runtime_thread_num = 0
            engine.batch_infer_size = 0
            engine.enable_batch_align = 0
            engine.model_data_path = model_config_path
            engine.enable_memory_optimization = self.memory_optimization
            engine.enable_ir_optimization = self.ir_optimization
            engine.static_optimization = False
            engine.force_update_static_cache = False
            if os.path.exists('{}/__params__'.format(model_config_path)):
                suffix = ""
            else:
                suffix = "_DIR"

            if device == "cpu":
                if self.encryption_model:
                    engine.type = "FLUID_CPU_ANALYSIS_ENCRYPT"
                else:
                    engine.type = "FLUID_CPU_ANALYSIS" + suffix
            elif device == "gpu":
                if self.encryption_model:
                    engine.type = "FLUID_GPU_ANALYSIS_ENCRYPT"
                else:
                    engine.type = "FLUID_GPU_ANALYSIS" + suffix
            self.model_toolkit_conf.append(server_sdk.ModelToolkitConf())
            self.model_toolkit_conf[-1].engines.extend([engine])

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
            for idx, op_general_model_config_fn in enumerate(self.general_model_config_fn):
                with open("{}/{}".format(workdir, op_general_model_config_fn),
                        "w") as fout:
                    fout.write(str(list(self.model_conf.values())[idx]))
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
                            if "quant" in node.name:
                                self.resource_conf.cube_quant_bits = 8
                self.resource_conf.model_toolkit_path.extend([workdir])
                self.resource_conf.model_toolkit_file.extend([self.model_toolkit_fn[idx]])
                self.resource_conf.general_model_path.extend([workdir])
                self.resource_conf.general_model_file.extend([op_general_model_config_fn])
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
        if isinstance(model_config_paths_args, list):
            # If there is only one model path, use the default infer_op.
            # Because there are several infer_op type, we need to find
            # it from workflow_conf.
            default_engine_types = [
                'GeneralInferOp', 'GeneralDistKVInferOp',
                'GeneralDistKVQuantInferOp','GeneralYSLOp',
            ]
            model_config_paths_list_idx = 0
            for node in self.workflow_conf.workflows[0].nodes:
                if node.type in default_engine_types:
                    if node.name is None:
                        raise Exception(
                            "You have set the engine_name of Op. Please use the form {op: model_path} to configure model path"
                        )
                    
                    f = open("{}/serving_server_conf.prototxt".format(
                        model_config_paths_args[model_config_paths_list_idx]), 'r')
                    self.model_conf[node.name] = google.protobuf.text_format.Merge(str(f.read()), m_config.GeneralModelConfig())
                    self.model_config_paths[node.name] = model_config_paths_args[model_config_paths_list_idx]
                    self.general_model_config_fn.append(node.name+"/general_model.prototxt")
                    self.model_toolkit_fn.append(node.name+"/model_toolkit.prototxt")
                    self.subdirectory.append(node.name)
                    model_config_paths_list_idx += 1
                    if model_config_paths_list_idx == len(model_config_paths_args):
                        break
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
            raise Exception("The type of model_config_paths must be str or "
                            "dict({op: model_path}), not {}.".format(
                                type(model_config_paths_args)))

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
                device_version = "serving-cpu-avx-mkl-"
            else:
                device_version = "serving-cpu-avx-openblas-"
        else:
            if mkl_flag:
                print(
                    "Your CPU does not support AVX, server will running with noavx-openblas mode."
                )
            device_version = "serving-cpu-noavx-openblas-"
        return device_version

    def download_bin(self):
        os.chdir(self.module_path)
        need_download = False
        device_version = self.get_device_version()
        folder_name = device_version + serving_server_version
        tar_name = folder_name + ".tar.gz"
        bin_url = "https://paddle-serving.bj.bcebos.com/bin/" + tar_name
        self.server_path = os.path.join(self.module_path, folder_name)

        #acquire lock
        version_file = open("{}/version.py".format(self.module_path), "r")
        fcntl.flock(version_file, fcntl.LOCK_EX)

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
                       cube_conf=None):
        if workdir == None:
            workdir = "./tmp"
            os.system("mkdir {}".format(workdir))
        else:
            os.system("mkdir {}".format(workdir))
        for subdir in self.subdirectory:
            os.system("mkdir {}/{}".format(workdir, subdir))
            os.system("touch {}/{}/fluid_time_file".format(workdir, subdir))

        if not self.port_is_available(port):
            raise SystemExit("Port {} is already used".format(port))
        self.set_port(port)
        self._prepare_resource(workdir, cube_conf)
        self._prepare_engine(self.model_config_paths, device)
        self._prepare_infer_service(port)
        self.workdir = workdir

        infer_service_fn = "{}/{}".format(workdir, self.infer_service_fn)
        self._write_pb_str(infer_service_fn, self.infer_service_conf)

        workflow_fn = "{}/{}".format(workdir, self.workflow_fn)
        self._write_pb_str(workflow_fn, self.workflow_conf)

        resource_fn = "{}/{}".format(workdir, self.resource_fn)
        self._write_pb_str(resource_fn, self.resource_conf)

        for idx,single_model_toolkit_fn in enumerate(self.model_toolkit_fn):
            model_toolkit_fn = "{}/{}".format(workdir, single_model_toolkit_fn)
            self._write_pb_str(model_toolkit_fn, self.model_toolkit_conf[idx])
        

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
        else:
            print("Use local bin : {}".format(self.bin_path))
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
        print("Going to Run Command")
        print(command)
        os.system(command)
