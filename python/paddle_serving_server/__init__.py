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
import fcntl

import numpy as np
import grpc
from .proto import multi_lang_general_model_service_pb2 as pb2
import sys
sys.path.append(
    os.path.join(os.path.abspath(os.path.dirname(__file__)), 'proto'))
from .proto import multi_lang_general_model_service_pb2_grpc as grpc_pb2
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
            "general_copy": "GeneralCopyOp"
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
                        'You must add op in order in OpSeqMaker. The previous op is {}, but the current op is followed by {}.'.
                        format(node.dependencies[0].name, self.workflow.nodes[
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
        self.num_threads = 4
        self.port = 8080
        self.reload_interval_s = 10
        self.max_body_size = 64 * 1024 * 1024
        self.module_path = os.path.dirname(paddle_serving_server.__file__)
        self.cur_path = os.getcwd()
        self.use_local_bin = False
        self.mkl_flag = False
        self.model_config_paths = None  # for multi-model in a workflow

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

    def check_local_bin(self):
        if "SERVING_BIN" in os.environ:
            self.use_local_bin = True
            self.bin_path = os.environ["SERVING_BIN"]

    def _prepare_engine(self, model_config_paths, device):
        if self.model_toolkit_conf == None:
            self.model_toolkit_conf = server_sdk.ModelToolkitConf()

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

            if device == "cpu":
                engine.type = "FLUID_CPU_ANALYSIS_DIR"
            elif device == "gpu":
                engine.type = "FLUID_GPU_ANALYSIS_DIR"

            self.model_toolkit_conf.engines.extend([engine])

    def _prepare_infer_service(self, port):
        if self.infer_service_conf == None:
            self.infer_service_conf = server_sdk.InferServiceConf()
            self.infer_service_conf.port = port
            infer_service = server_sdk.InferService()
            infer_service.name = "GeneralModelService"
            infer_service.workflows.extend(["workflow1"])
            self.infer_service_conf.services.extend([infer_service])

    def _prepare_resource(self, workdir):
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
                        if "quant" in node.name:
                            self.resource_conf.cube_quant_bits = 8
            self.resource_conf.model_toolkit_path = workdir
            self.resource_conf.model_toolkit_file = self.model_toolkit_fn
            self.resource_conf.general_model_path = workdir
            self.resource_conf.general_model_file = self.general_model_config_fn

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
        floder_name = device_version + serving_server_version
        tar_name = floder_name + ".tar.gz"
        bin_url = "https://paddle-serving.bj.bcebos.com/bin/" + tar_name
        self.server_path = os.path.join(self.module_path, floder_name)

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
                    'Download failed, please check your network or permission of {}.'.
                    format(self.module_path))
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
                        'Decompressing failed, please check your permission of {} or disk space left.'.
                        foemat(self.module_path))
                finally:
                    os.remove(tar_name)
        #release lock
        version_file.close()
        os.chdir(self.cur_path)
        self.bin_path = self.server_path + "/serving"

    def prepare_server(self, workdir=None, port=9292, device="cpu"):
        if workdir == None:
            workdir = "./tmp"
            os.system("mkdir {}".format(workdir))
        else:
            os.system("mkdir {}".format(workdir))
        os.system("touch {}/fluid_time_file".format(workdir))

        if not self.port_is_available(port):
            raise SystemExit("Prot {} is already used".format(port))
        self._prepare_resource(workdir)
        self._prepare_engine(self.model_config_paths, device)
        self._prepare_infer_service(port)
        self.port = port
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


class MultiLangServerService(
        grpc_pb2.MultiLangGeneralModelService):
    def __init__(self, model_config_path, endpoints):
        from paddle_serving_client import Client

        path = "{}/serving_server_conf.prototxt".format(model_config_path)
        with open(path, 'r') as f:
            proto_txt = str(f.read())

        self._parse_model_config(proto_txt)
        self.bclient_ = Client()
        self.bclient_.load_client_config(path)
        self.bclient_.connect(endpoints)

        self._max_batch_size = -1 #  <=0:infinite
        self._proto_txt = proto_txt

    def _parse_model_config(self, proto_txt):
        model_conf = m_config.GeneralModelConfig()
        model_conf = google.protobuf.text_format.Merge(proto_txt), model_conf)
        self.feed_names_ = [var.alias_name for var in model_conf.feed_var]
        self.feed_types_ = {}
        self.feed_shapes_ = {}
        self.fetch_names_ = [var.alias_name for var in model_conf.fetch_var]
        self.fetch_types_ = {}
        self.lod_tensor_set_ = set()
        for i, var in enumerate(model_conf.feed_var):
            self.feed_types_[var.alias_name] = var.feed_type
            self.feed_shapes_[var.alias_name] = var.shape
            if var.is_lod_tensor:
                self.lod_tensor_set_.add(var.alias_name)
        for i, var in enumerate(model_conf.fetch_var):
            self.fetch_types_[var.alias_name] = var.fetch_type
            if var.is_lod_tensor:
                self.lod_tensor_set_.add(var.alias_name)

    def _flatten_list(self, nested_list):
        for item in nested_list:
            if isinstance(item, (list, tuple)):
                for sub_item in self._flatten_list(item):
                    yield sub_item
            else:
                yield item

    def _unpack_request(self, request):
        feed_names = list(request.feed_var_names)
        fetch_names = list(request.fetch_var_names)
        is_python = request.is_python
        feed_batch = []
        for feed_inst in request.insts:
            feed_dict = {}
            for idx, name in enumerate(feed_names):
                var = feed_inst.tensor_array[idx]
                v_type = self.feed_types_[name]
                data = None
                if is_python:
                    if v_type == 0:
                        data = np.frombuffer(var.data, dtype="int64")
                    elif v_type == 1:
                        data = np.frombuffer(var.data, dtype="float32")
                    else:
                        raise Exception("error type.")
                else:
                    if v_type == 0:  # int64
                        data = np.array(list(var.int64_data), dtype="int64")
                    elif v_type == 1:  # float32
                        data = np.array(list(var.float_data), dtype="float32")
                    else:
                        raise Exception("error type.")
                data.shape = list(feed_inst.tensor_array[idx].shape)
                feed_dict[name] = data
            feed_batch.append(feed_dict)
        return feed_batch, fetch_names, is_python

    def _pack_resp_package(self, result, fetch_names, is_python, tag):
        resp = pb2.Response()
        # Only one model is supported temporarily
        model_output = pb2.ModelOutput()
        inst = pb2.FetchInst()
        for idx, name in enumerate(fetch_names):
            tensor = pb2.Tensor()
            v_type = self.fetch_types_[name]
            if is_python:
                tensor.data = result[name].tobytes()
            else:
                if v_type == 0:  # int64
                    tensor.int64_data.extend(result[name].reshape(-1).tolist())
                elif v_type == 1:  # float32
                    tensor.float_data.extend(result[name].reshape(-1).tolist())
                else:
                    raise Exception("error type.")
            tensor.shape.extend(list(result[name].shape))
            if name in self.lod_tensor_set_:
                tensor.lod.extend(result["{}.lod".format(name)].tolist())
            inst.tensor_array.append(tensor)
        model_output.insts.append(inst)
        resp.outputs.append(model_output)
        resp.tag = tag
        return resp

    def inference(self, request, context):
        feed_dict, fetch_names, is_python = self._unpack_request(request)
        data, tag = self.bclient_.predict(
            feed=feed_dict, fetch=fetch_names, need_variant_tag=True)
        return self._pack_resp_package(data, fetch_names, is_python, tag)

    
    def get_config(self, request, context):
        key = "PADDLE_SERVING_MAX_BATCH_SIZE"
        max_batch_size = os.getenv(key)
        if max_batch_size:
            try:
                max_batch_size=int(max_batch_size)
                self._max_batch_size = max_batch_size
            except Exception as e:
                print("invalid value:{} of {}".format(max_batch_size, key))

        response = pb2.ServingConfig()
        response.proto_txt = self.proto_txt
        response.max_batch_size = self._max_batch_size

        return response

class MultiLangServer(object):
    def __init__(self, worker_num=2):
        self.bserver_ = Server()
        self.worker_num_ = worker_num

    def set_op_sequence(self, op_seq):
        self.bserver_.set_op_sequence(op_seq)

    def load_model_config(self, model_config_path):
        if not isinstance(model_config_path, str):
            raise Exception(
                "MultiLangServer only supports multi-model temporarily")
        self.bserver_.load_model_config(model_config_path)
        self.model_config_path_ = model_config_path

    def prepare_server(self, workdir=None, port=9292, device="cpu"):
        default_port = 12000
        self.port_list_ = []
        for i in range(1000):
            if default_port + i != port and self._port_is_available(default_port
                                                                    + i):
                self.port_list_.append(default_port + i)
                break
        self.bserver_.prepare_server(
            workdir=workdir, port=self.port_list_[0], device=device)
        self.gport_ = port

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
        server = grpc.server(
            futures.ThreadPoolExecutor(max_workers=self.worker_num_))
        pb2_grpc.add_MultiLangGeneralModelServiceServicer_to_server(
            MultiLangServerService(self.model_config_path_,
                                   ["0.0.0.0:{}".format(self.port_list_[0])]),
            server)
        server.add_insecure_port('[::]:{}'.format(self.gport_))
        server.start()
        p_bserver.join()
        server.wait_for_termination()
