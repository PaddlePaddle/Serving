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

from ..proto import server_configure_pb2 as server_sdk

class OpMaker(object):
    def __init__(self):
        self.op_dict = {"general_model":"GeneralModelOp",
                        "general_reader":"GeneralReaderOp",
                        "general_single_kv":"GeneralSingleKVOp",
                        "general_dist_kv":"GeneralDistKVOp"}

    def create(self, name, inputs=[], outputs=[]):
        if name not in self.op_dict:
            raise Exception("Op name {} is not supported right now".format(name))
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
        self.engine = None
        self.workflow_fn = "workflow.prototxt"
        self.resource_fn = "resource.prototxt"
        self.infer_service_fn = "infer_service.prototxt"
        self.model_toolkit_fn = "model_toolkit.prototxt"
        self.workdir = ""
        self.max_concurrency = 0
        self.num_threads = 0
        self.port = 8080
        self.reload_interval_s = 10

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

    def _prepare_engine(self, model_config_path, device):
        if self.model_toolkit_conf == None:
            self.model_toolkit_conf = server_sdk.ModelToolkitConf()
        if self.engine == None:
            self.engine = server_sdk.EngineDesc()
        self.engine.name = "general_model"
        self.engine.reloadable_type = "tiemstamp_ne"
        self.engine.runtime_thread_num = 0
        self.engine.batch_infer_size = 0
        self.engine.enable_batch_align = 0
        self.engine.model_data_path = model_path
        if device == "cpu":
            self.engine.type = "FLUID_CPU_ANALYSIS_DIR"
        elif device == "gpu":
            self.engine.type = "FLUID_GPU_ANALYSIS_DIR"
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
            self.resource_conf = server_sdk.ResourceConf()
            self.resource_conf.model_toolkit_path = workdir
            self.resource_conf.model_toolkit_file = "server_model_toolkit.prototxt"

    def _write_pb_str(self, filepath, pb_obj):
        with open(filepath, "w") as fout:
            fout.write(str(pb_obj))

    def load_model_config(self, path):
        self.config_file = "{}/inference.conf".format(path)
        # check config here

    def prepare_server(self, port=9292, device="cpu", workdir=None):
        if workdir == None:
            workdir = "./tmp"
            os.system("mkdir {}".format(workdir))
        else:
            os.system("mkdir {}".format(workdir))

        self._prepare_resource(workdir)
        self._prepare_engine(self.config_file, device)
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

    def run_server(self):
        # just run server with system command
        # currently we do not load cube
        command = "./pdserving -enable_model_toolkit " \
                  "-inferservice_path {} " \
                  "-inferservice_file {} " \
                  "-max_concurrency {} " \
                  "-num_threads {} " \
                  "-port {} " \
                  "-reload_interval_s {} " \
                  "-resource_path {} " \
                  "-resource_file {} " \
                  "-workflow_path {} " \
                  "-workflow_file {} ".format(
                      self.workdir,
                      self.infer_service_fn,
                      self.max_concurrency,
                      self.num_threads,
                      self.port,
                      self.reload_interval_s,
                      self.workdir,
                      self.resource_fn,
                      self.workdir, 
                      self.workflow_fn)
        os.system(command)


