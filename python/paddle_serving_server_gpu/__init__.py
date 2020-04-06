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


def serve_args():
    parser = argparse.ArgumentParser("serve")
    parser.add_argument(
        "--thread", type=int, default=10, help="Concurrency of server")
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
        "--mem_optim", type=bool, default=False, help="Memory optimize")
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

    # currently, inputs and outputs are not used
    # when we have OpGraphMaker, inputs and outputs are necessary
    def create(self, name, inputs=[], outputs=[]):
        if name not in self.op_dict:
            raise Exception("Op name {} is not supported right now".format(
                name))
        node = server_sdk.DAGNode()
        node.name = "{}_op".format(name)
        node.type = self.op_dict[name]
        return node


class OpSeqMaker(object):
    def __init__(self):
        self.workflow = server_sdk.Workflow()
        self.workflow.name = "workflow1"
        self.workflow.workflow_type = "Sequence"

    def add_op(self, node):
        if len(self.workflow.nodes) >= 1:
            dep = server_sdk.DAGNodeDependency()
            dep.name = self.workflow.nodes[-1].name
            dep.mode = "RO"
            node.dependencies.extend([dep])
        self.workflow.nodes.extend([node])

    def get_op_sequence(self):
        workflow_conf = server_sdk.WorkflowConf()
        workflow_conf.workflows.extend([self.workflow])
        return workflow_conf


class Server(object):
    def __init__(self):
        self.server_handle_ = None
        self.infer_service_conf = None
        self.model_toolkit_conf = None
        self.resource_conf = None
        self.engine = None
        self.memory_optimization = False
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
        self.module_path = os.path.dirname(paddle_serving_server.__file__)
        self.cur_path = os.getcwd()
        self.check_cuda()
        self.use_local_bin = False
        self.gpuid = 0

    def set_max_concurrency(self, concurrency):
        self.max_concurrency = concurrency

    def set_num_threads(self, threads):
        self.num_threads = threads

    def set_port(self, port):
        self.port = port

    def set_reload_interval(self, interval):
        self.reload_interval_s = interval

    def set_op_sequence(self, op_seq):
        self.workflow_conf = op_seq

    def set_memory_optimize(self, flag=False):
        self.memory_optimization = flag

    def check_local_bin(self):
        if "SERVING_BIN" in os.environ:
            self.use_local_bin = True
            self.bin_path = os.environ["SERVING_BIN"]

    def check_cuda(self):
        r = os.system("nvcc --version > /dev/null")
        if r != 0:
            raise SystemExit(
                "CUDA not found, please check your environment or use cpu version by \"pip install paddle_serving_server\""
            )

    def set_gpuid(self, gpuid=0):
        self.gpuid = gpuid

    def _prepare_engine(self, model_config_path, device):
        if self.model_toolkit_conf == None:
            self.model_toolkit_conf = server_sdk.ModelToolkitConf()

        if self.engine == None:
            self.engine = server_sdk.EngineDesc()

        self.model_config_path = model_config_path
        self.engine.name = "general_model"
        #self.engine.reloadable_meta = model_config_path + "/fluid_time_file"
        self.engine.reloadable_meta = self.workdir + "/fluid_time_file"
        os.system("touch {}".format(self.engine.reloadable_meta))
        self.engine.reloadable_type = "timestamp_ne"
        self.engine.runtime_thread_num = 0
        self.engine.batch_infer_size = 0
        self.engine.enable_batch_align = 0
        self.engine.model_data_path = model_config_path
        self.engine.enable_memory_optimization = self.memory_optimization
        self.engine.static_optimization = False
        self.engine.force_update_static_cache = False

        if device == "cpu":
            self.engine.type = "FLUID_CPU_ANALYSIS_DIR"
        elif device == "gpu":
            self.engine.type = "FLUID_GPU_ANALYSIS_DIR"

        self.model_toolkit_conf.engines.extend([self.engine])

    def _prepare_infer_service(self, port):
        if self.infer_service_conf == None:
            self.infer_service_conf = server_sdk.InferServiceConf()
            self.infer_service_conf.port = port
            infer_service = server_sdk.InferService()
            infer_service.name = "GeneralModelService"
            infer_service.workflows.extend(["workflow1"])
            self.infer_service_conf.services.extend([infer_service])

    def _prepare_resource(self, workdir):
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
            self.resource_conf.model_toolkit_path = workdir
            self.resource_conf.model_toolkit_file = self.model_toolkit_fn
            self.resource_conf.general_model_path = workdir
            self.resource_conf.general_model_file = self.general_model_config_fn

    def _write_pb_str(self, filepath, pb_obj):
        with open(filepath, "w") as fout:
            fout.write(str(pb_obj))

    def load_model_config(self, path):
        self.model_config_path = path
        self.model_conf = m_config.GeneralModelConfig()
        f = open("{}/serving_server_conf.prototxt".format(path), 'r')
        self.model_conf = google.protobuf.text_format.Merge(
            str(f.read()), self.model_conf)
        # check config here
        # print config here

    def download_bin(self):
        os.chdir(self.module_path)
        need_download = False
        device_version = "serving-gpu-"
        folder_name = device_version + serving_server_version
        tar_name = folder_name + ".tar.gz"
        bin_url = "https://paddle-serving.bj.bcebos.com/bin/" + tar_name
        self.server_path = os.path.join(self.module_path, folder_name)

        download_flag = "{}/{}.is_download".format(self.module_path,
                                                   folder_name)
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

        self.set_port(port)
        self._prepare_resource(workdir)
        self._prepare_engine(self.model_config_path, device)
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
                  "-gpuid {} ".format(
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
                      self.gpuid,)
        print("Going to Run Comand")
        print(command)
        # wait for other process to download server bin
        while not os.path.exists(self.server_path):
            time.sleep(1)

        os.system(command)
