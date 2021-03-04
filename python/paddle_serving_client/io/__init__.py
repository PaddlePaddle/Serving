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
from paddle.fluid.core import CipherUtils
from paddle.fluid.core import CipherFactory
from paddle.fluid.core import Cipher
from ..proto import general_model_config_pb2 as model_conf
import os
import paddle
import paddle.nn.functional as F
import errno
from paddle.jit import to_static


def save_dygraph_model(serving_model_folder, client_config_folder, model):
    paddle.jit.save(model, "serving_tmp")
    loaded_layer = paddle.jit.load(
        path=".",
        model_filename="serving_tmp.pdmodel",
        params_filename="serving_tmp.pdiparams")
    feed_target_names = [x.name for x in loaded_layer._input_spec()]
    fetch_target_names = [x.name for x in loaded_layer._output_spec()]

    inference_program = loaded_layer.program()
    feed_var_dict = {
        x: inference_program.global_block().var(x)
        for x in feed_target_names
    }
    fetch_var_dict = {
        x: inference_program.global_block().var(x)
        for x in fetch_target_names
    }
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
    for key in fetch_var_dict:
        fetch_var = model_conf.FetchVar()
        fetch_var.alias_name = key
        fetch_var.name = fetch_var_dict[key].name
        fetch_var.is_lod_tensor = 1
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
    cmd = "mkdir -p {}".format(serving_model_folder)
    os.system(cmd)
    cmd = "mv {} {}/__model__".format("serving_tmp.pdmodel",
                                      serving_model_folder)
    os.system(cmd)
    cmd = "mv {} {}/__params__".format("serving_tmp.pdiparams",
                                       serving_model_folder)
    os.system(cmd)
    cmd = "rm -rf serving_tmp.pd*"
    os.system(cmd)
    with open("{}/serving_client_conf.prototxt".format(client_config_folder),
              "w") as fout:
        fout.write(str(config))
    with open("{}/serving_server_conf.prototxt".format(serving_model_folder),
              "w") as fout:
        fout.write(str(config))
    with open("{}/serving_client_conf.stream.prototxt".format(
            client_config_folder), "wb") as fout:
        fout.write(config.SerializeToString())
    with open("{}/serving_server_conf.stream.prototxt".format(
            serving_model_folder), "wb") as fout:
        fout.write(config.SerializeToString())


def save_model(server_model_folder,
               client_config_folder,
               feed_var_dict,
               fetch_var_dict,
               main_program=None,
               encryption=False,
               key_len=128,
               encrypt_conf=None):
    executor = Executor(place=CPUPlace())

    feed_var_names = [feed_var_dict[x].name for x in feed_var_dict]
    target_vars = []
    target_var_names = []
    for key in sorted(fetch_var_dict.keys()):
        target_vars.append(fetch_var_dict[key])
        target_var_names.append(key)

    if not encryption:
        save_inference_model(
            server_model_folder,
            feed_var_names,
            target_vars,
            executor,
            model_filename="__model__",
            params_filename="__params__",
            main_program=main_program)
    else:
        if encrypt_conf == None:
            aes_cipher = CipherFactory.create_cipher()
        else:
            #todo: more encryption algorithms
            pass
        key = CipherUtils.gen_key_to_file(128, "key")
        params = fluid.io.save_persistables(
            executor=executor, dirname=None, main_program=main_program)
        model = main_program.desc.serialize_to_string()
        if not os.path.exists(server_model_folder):
            os.makedirs(server_model_folder)
        os.chdir(server_model_folder)
        aes_cipher.encrypt_to_file(params, key, "encrypt_params")
        aes_cipher.encrypt_to_file(model, key, "encrypt_model")
        os.chdir("..")

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
        #fetch_var.is_lod_tensor = fetch_var_dict[key].lod_level >= 1
        fetch_var.is_lod_tensor = 1
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

    try:
        save_dirname = os.path.normpath(client_config_folder)
        os.makedirs(save_dirname)
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise
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
                               params_filename=None,
                               encryption=False,
                               key_len=128,
                               encrypt_conf=None):
    paddle.enable_static()
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
               inference_program, encryption, key_len, encrypt_conf)
    feed_names = feed_dict.keys()
    fetch_names = fetch_dict.keys()
    return feed_names, fetch_names
