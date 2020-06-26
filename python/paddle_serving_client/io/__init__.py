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

from paddle.fluid import Executor
from paddle.fluid.compiler import CompiledProgram
from paddle.fluid.framework import core
from paddle.fluid.framework import default_main_program
from paddle.fluid.framework import Program
from paddle.fluid import CPUPlace
from paddle.fluid.io import save_inference_model
import paddle.fluid as fluid
from ..proto import general_model_config_pb2 as model_conf
import os


def save_model(server_model_folder,
               client_config_folder,
               feed_var_dict,
               fetch_var_dict,
               main_program=None):
    executor = Executor(place=CPUPlace())

    feed_var_names = [feed_var_dict[x].name for x in feed_var_dict]
    target_vars = []
    target_var_names = []
    for key in sorted(fetch_var_dict.keys()):
        target_vars.append(fetch_var_dict[key])
        target_var_names.append(key)

    save_inference_model(
        server_model_folder,
        feed_var_names,
        target_vars,
        executor,
        main_program=main_program)

    config = model_conf.GeneralModelConfig()

    #int64 = 0; float32 = 1; int32 = 2;
    for key in feed_var_dict:
        feed_var = model_conf.FeedVar()
        feed_var.alias_name = key
        feed_var.name = feed_var_dict[key].name
        feed_var.is_lod_tensor = feed_var_dict[key].lod_level >= 1
        if feed_var_dict[key].dtype == core.VarDesc.VarType.INT64:
            feed_var.feed_type = 0
        if feed_var_dict[key].dtype == core.VarDesc.VarType.FP32:
            feed_var.feed_type = 1
        if feed_var_dict[key].dtype == core.VarDesc.VarType.INT32:
            feed_var.feed_type = 2
        if feed_var.is_lod_tensor:
            feed_var.shape.extend([-1])
        else:
            tmp_shape = []
            for v in feed_var_dict[key].shape:
                if v >= 0:
                    tmp_shape.append(v)
            feed_var.shape.extend(tmp_shape)
        config.feed_var.extend([feed_var])

    for key in target_var_names:
        fetch_var = model_conf.FetchVar()
        fetch_var.alias_name = key
        fetch_var.name = fetch_var_dict[key].name
        fetch_var.is_lod_tensor = fetch_var_dict[key].lod_level >= 1
        if fetch_var_dict[key].dtype == core.VarDesc.VarType.INT64:
            fetch_var.fetch_type = 0
        if fetch_var_dict[key].dtype == core.VarDesc.VarType.FP32:
            fetch_var.fetch_type = 1
        if fetch_var_dict[key].dtype == core.VarDesc.VarType.INT32:
            fetch_var.fetch_type = 2
        if fetch_var.is_lod_tensor:
            fetch_var.shape.extend([-1])
        else:
            tmp_shape = []
            for v in fetch_var_dict[key].shape:
                if v >= 0:
                    tmp_shape.append(v)
            fetch_var.shape.extend(tmp_shape)
        config.fetch_var.extend([fetch_var])

    cmd = "mkdir -p {}".format(client_config_folder)

    os.system(cmd)
    with open("{}/serving_client_conf.prototxt".format(client_config_folder),
              "w") as fout:
        fout.write(str(config))
    with open("{}/serving_server_conf.prototxt".format(server_model_folder),
              "w") as fout:
        fout.write(str(config))
    with open("{}/serving_client_conf.stream.prototxt".format(
            client_config_folder), "wb") as fout:
        fout.write(config.SerializeToString())
    with open("{}/serving_server_conf.stream.prototxt".format(
            server_model_folder), "wb") as fout:
        fout.write(config.SerializeToString())


def inference_model_to_serving(dirname,
                               serving_server="serving_server",
                               serving_client="serving_client",
                               model_filename=None,
                               params_filename=None):
    place = fluid.CPUPlace()
    exe = fluid.Executor(place)
    inference_program, feed_target_names, fetch_targets = \
            fluid.io.load_inference_model(dirname=dirname, executor=exe, model_filename=model_filename, params_filename=params_filename)
    feed_dict = {
        x: inference_program.global_block().var(x)
        for x in feed_target_names
    }
    fetch_dict = {x.name: x for x in fetch_targets}
    save_model(serving_server, serving_client, feed_dict, fetch_dict,
               inference_program)
    feed_names = feed_dict.keys()
    fetch_names = fetch_dict.keys()
    return feed_names, fetch_names
