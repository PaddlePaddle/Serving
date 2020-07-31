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
import paddle_serving_server_gpu as paddle_serving_server
import time
from .version import serving_server_version
from contextlib import closing
import argparse
import collections
import fcntl
import shutil
import numpy as np
import grpc
from .proto import multi_lang_general_model_service_pb2
import sys
sys.path.append(
    os.path.join(os.path.abspath(os.path.dirname(__file__)), 'proto'))
from .proto import multi_lang_general_model_service_pb2_grpc
from multiprocessing import Pool, Process
from concurrent import futures


def serve_args():
    parser = argparse.ArgumentParser("serve")
    parser.add_argument(
        "--thread", type=int, default=2, help="Concurrency of server")
    parser.add_argument(
        "--model", type=str, default="", help="Model for serving")
    parser.add_argument(
        "--port", type=int, default=9292, help="Port of the starting gpu")
    parser.add_argument(
        "--workdir",
        type=str,
        default="workdir",
        help="Working dir of current service")
    parser.add_argument(
        "--device", type=str, default="gpu", help="Type of device")
    parser.add_argument("--gpu_ids", type=str, default="", help="gpu ids")
    parser.add_argument(
        "--name", type=str, default="None", help="Default service name")
    parser.add_argument(
        "--mem_optim_off",
        default=False,
        action="store_true",
        help="Memory optimize")
    parser.add_argument(
        "--ir_optim", default=False, action="store_true", help="Graph optimize")
    parser.add_argument(
        "--max_body_size",
        type=int,
        default=512 * 1024 * 1024,
        help="Limit sizes of messages")
    parser.add_argument(
        "--use_multilang",
        default=False,
        action="store_true",
        help="Use Multi-language-service")
    return parser.parse_args()


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
            "general_dist_kv": "GeneralDistKVOp"
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
        self.num_threads = 2
        self.port = 8080
        self.reload_interval_s = 10
        self.max_body_size = 64 * 1024 * 1024
        self.module_path = os.path.dirname(paddle_serving_server.__file__)
        self.cur_path = os.getcwd()
        self.use_local_bin = False
        self.gpuid = 0
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

    def check_cuda(self):
        cuda_flag = False
        r = os.popen("ldd {} | grep cudart".format(self.bin_path))
        r = r.read().split("=")
        if len(r) >= 2 and "cudart" in r[1] and os.system(
                "ls /dev/ | grep nvidia > /dev/null") == 0:
            cuda_flag = True
        if not cuda_flag:
            raise SystemExit(
                "CUDA not found, please check your environment or use cpu version by \"pip install paddle_serving_server\""
            )

    def set_gpuid(self, gpuid=0):
        self.gpuid = gpuid

    def _prepare_engine(self, model_config_paths, device):
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

    def download_bin(self):
        os.chdir(self.module_path)
        need_download = False

        #acquire lock
        version_file = open("{}/version.py".format(self.module_path), "r")
        import re
        for line in version_file.readlines():
            if re.match("cuda_version", line):
                cuda_version = line.split("\"")[1]
                device_version = "serving-gpu-cuda" + cuda_version + "-"

        folder_name = device_version + serving_server_version
        tar_name = folder_name + ".tar.gz"
        bin_url = "https://paddle-serving.bj.bcebos.com/bin/" + tar_name
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
                        format(self.module_path))
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
        os.system("touch {}/fluid_time_file".format(workdir))

        if not self.port_is_available(port):
            raise SystemExit("Port {} is already used".format(port))

        self.set_port(port)
        self._prepare_resource(workdir, cube_conf)
        self._prepare_engine(self.model_config_paths, device)
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
        self.check_cuda()
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


class MultiLangServerServiceServicer(multi_lang_general_model_service_pb2_grpc.
                                     MultiLangGeneralModelServiceServicer):
    def __init__(self, model_config_path, is_multi_model, endpoints):
        self.is_multi_model_ = is_multi_model
        self.model_config_path_ = model_config_path
        self.endpoints_ = endpoints
        with open(self.model_config_path_) as f:
            self.model_config_str_ = str(f.read())
        self._parse_model_config(self.model_config_str_)
        self._init_bclient(self.model_config_path_, self.endpoints_)

    def _init_bclient(self, model_config_path, endpoints, timeout_ms=None):
        from paddle_serving_client import Client
        self.bclient_ = Client()
        if timeout_ms is not None:
            self.bclient_.set_rpc_timeout_ms(timeout_ms)
        self.bclient_.load_client_config(model_config_path)
        self.bclient_.connect(endpoints)

    def _parse_model_config(self, model_config_str):
        model_conf = m_config.GeneralModelConfig()
        model_conf = google.protobuf.text_format.Merge(model_config_str,
                                                       model_conf)
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

    def _unpack_inference_request(self, request):
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
                    elif v_type == 2:
                        data = np.frombuffer(var.data, dtype="int32")
                    else:
                        raise Exception("error type.")
                else:
                    if v_type == 0:  # int64
                        data = np.array(list(var.int64_data), dtype="int64")
                    elif v_type == 1:  # float32
                        data = np.array(list(var.float_data), dtype="float32")
                    elif v_type == 2:
                        data = np.array(list(var.int_data), dtype="int32")
                    else:
                        raise Exception("error type.")
                data.shape = list(feed_inst.tensor_array[idx].shape)
                feed_dict[name] = data
            feed_batch.append(feed_dict)
        return feed_batch, fetch_names, is_python

    def _pack_inference_response(self, ret, fetch_names, is_python):
        resp = multi_lang_general_model_service_pb2.InferenceResponse()
        if ret is None:
            resp.err_code = 1
            return resp
        results, tag = ret
        resp.tag = tag
        resp.err_code = 0

        if not self.is_multi_model_:
            results = {'general_infer_0': results}
        for model_name, model_result in results.items():
            model_output = multi_lang_general_model_service_pb2.ModelOutput()
            inst = multi_lang_general_model_service_pb2.FetchInst()
            for idx, name in enumerate(fetch_names):
                tensor = multi_lang_general_model_service_pb2.Tensor()
                v_type = self.fetch_types_[name]
                if is_python:
                    tensor.data = model_result[name].tobytes()
                else:
                    if v_type == 0:  # int64
                        tensor.int64_data.extend(model_result[name].reshape(-1)
                                                 .tolist())
                    elif v_type == 1:  # float32
                        tensor.float_data.extend(model_result[name].reshape(-1)
                                                 .tolist())
                    elif v_type == 2:  # int32
                        tensor.int_data.extend(model_result[name].reshape(-1)
                                               .tolist())
                    else:
                        raise Exception("error type.")
                tensor.shape.extend(list(model_result[name].shape))
                if name in self.lod_tensor_set_:
                    tensor.lod.extend(model_result["{}.lod".format(name)]
                                      .tolist())
                inst.tensor_array.append(tensor)
            model_output.insts.append(inst)
            model_output.engine_name = model_name
            resp.outputs.append(model_output)
        return resp

    def SetTimeout(self, request, context):
        # This porcess and Inference process cannot be operate at the same time.
        # For performance reasons, do not add thread lock temporarily.
        timeout_ms = request.timeout_ms
        self._init_bclient(self.model_config_path_, self.endpoints_, timeout_ms)
        resp = multi_lang_general_model_service_pb2.SimpleResponse()
        resp.err_code = 0
        return resp

    def Inference(self, request, context):
        feed_dict, fetch_names, is_python = self._unpack_inference_request(
            request)
        ret = self.bclient_.predict(
            feed=feed_dict, fetch=fetch_names, need_variant_tag=True)
        return self._pack_inference_response(ret, fetch_names, is_python)

    def GetClientConfig(self, request, context):
        resp = multi_lang_general_model_service_pb2.GetClientConfigResponse()
        resp.client_config_str = self.model_config_str_
        return resp


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

    def set_port(self, port):
        self.gport_ = port

    def set_reload_interval(self, interval):
        self.bserver_.set_reload_interval(interval)

    def set_op_sequence(self, op_seq):
        self.bserver_.set_op_sequence(op_seq)

    def set_op_graph(self, op_graph):
        self.bserver_.set_op_graph(op_graph)

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
