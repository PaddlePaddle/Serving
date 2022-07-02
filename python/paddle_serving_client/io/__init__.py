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
from .paddle_io import save_inference_model, normalize_program
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

_PADDLE_DTYPE_2_NUMPY_DTYPE = {
    core.VarDesc.VarType.BOOL: 'bool',
    core.VarDesc.VarType.FP16: 'float16',
    core.VarDesc.VarType.BF16: 'uint16',
    core.VarDesc.VarType.FP32: 'float32',
    core.VarDesc.VarType.FP64: 'float64',
    core.VarDesc.VarType.INT8: 'int8',
    core.VarDesc.VarType.INT16: 'int16',
    core.VarDesc.VarType.INT32: 'int32',
    core.VarDesc.VarType.INT64: 'int64',
    core.VarDesc.VarType.UINT8: 'uint8',
    core.VarDesc.VarType.COMPLEX64: 'complex64',
    core.VarDesc.VarType.COMPLEX128: 'complex128',
}


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

    for key in feed_var_dict:
        feed_var = model_conf.FeedVar()
        feed_var.alias_name = key
        feed_var.name = feed_var_dict[key].name
        feed_var.feed_type = var_type_conversion(feed_var_dict[key].dtype)
        feed_var.is_lod_tensor = feed_var_dict[key].lod_level >= 1
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
        fetch_var.fetch_type = var_type_conversion(fetch_var_dict[key].dtype)
        fetch_var.is_lod_tensor = 1
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


def var_type_conversion(dtype):
    """
    Variable type conversion
    Args:
        dtype: type of core.VarDesc.VarType.xxxxx
        (https://github.com/PaddlePaddle/Paddle/blob/release/2.1/python/paddle/framework/dtype.py) 
    
    Returns:
        (int)type value, -1 is type matching failed.
            int64 => 0; 
            float32 => 1; 
            int32 => 2; 
            float64 => 3; 
            int16 => 4; 
            float16 => 5; 
            bfloat16 => 6; 
            uint8 => 7; 
            int8 => 8; 
            bool => 9;
            complex64 => 10, 
            complex128 => 11;
    """
    type_val = -1
    if dtype == core.VarDesc.VarType.INT64:
        type_val = 0
    elif dtype == core.VarDesc.VarType.FP32:
        type_val = 1
    elif dtype == core.VarDesc.VarType.INT32:
        type_val = 2
    elif dtype == core.VarDesc.VarType.FP64:
        type_val = 3
    elif dtype == core.VarDesc.VarType.INT16:
        type_val = 4
    elif dtype == core.VarDesc.VarType.FP16:
        type_val = 5
    elif dtype == core.VarDesc.VarType.BF16:
        type_val = 6
    elif dtype == core.VarDesc.VarType.UINT8:
        type_val = 7
    elif dtype == core.VarDesc.VarType.INT8:
        type_val = 8
    elif dtype == core.VarDesc.VarType.BOOL:
        type_val = 9
    elif dtype == core.VarDesc.VarType.COMPLEX64:
        type_val = 10
    elif dtype == core.VarDesc.VarType.COMPLEX128:
        type_val = 11
    else:
        type_val = -1
    return type_val


def save_model(server_model_folder,
               client_config_folder,
               feed_var_dict,
               fetch_var_dict,
               main_program=None,
               encryption=False,
               key_len=128,
               encrypt_conf=None,
               model_filename=None,
               params_filename=None,
               show_proto=False,
               feed_alias_names=None,
               fetch_alias_names=None):
    executor = Executor(place=CPUPlace())

    feed_var_names = [feed_var_dict[x].name for x in feed_var_dict]
    feed_vars = [feed_var_dict[x] for x in feed_var_dict]
    target_vars = []
    target_var_names = []
    for key in sorted(fetch_var_dict.keys()):
        target_vars.append(fetch_var_dict[key])
        target_var_names.append(key)

    main_program = normalize_program(main_program, feed_vars, target_vars)
    if not encryption and not show_proto:
        if not os.path.exists(server_model_folder):
            os.makedirs(server_model_folder)
        if not model_filename:
            model_filename = "model.pdmodel"
        if not params_filename:
            params_filename = "params.pdiparams"

        new_model_path = os.path.join(server_model_folder, model_filename)
        new_params_path = os.path.join(server_model_folder, params_filename)

        with open(new_model_path, "wb") as new_model_file:
            new_model_file.write(
                main_program._remove_training_info(False)
                .desc.serialize_to_string())

        paddle.static.save_vars(
            executor=executor,
            dirname=server_model_folder,
            main_program=main_program,
            vars=None,
            predicate=paddle.static.io.is_persistable,
            filename=params_filename)
    elif not show_proto:
        if not os.path.exists(server_model_folder):
            os.makedirs(server_model_folder)
        if encrypt_conf == None:
            aes_cipher = CipherFactory.create_cipher()
        else:
            #todo: more encryption algorithms
            pass
        key = CipherUtils.gen_key_to_file(128, "key")
        params = fluid.io.save_persistables(
            executor=executor, dirname=None, main_program=main_program)
        model = main_program._remove_training_info(
            False).desc.serialize_to_string()
        if not os.path.exists(server_model_folder):
            os.makedirs(server_model_folder)
        os.chdir(server_model_folder)
        aes_cipher.encrypt_to_file(params, key, "encrypt_params")
        aes_cipher.encrypt_to_file(model, key, "encrypt_model")
        os.chdir("..")

    config = model_conf.GeneralModelConfig()
    if feed_alias_names is None:
        feed_alias = list(feed_var_dict.keys())
    else:
        feed_alias = feed_alias_names.split(',')
    if fetch_alias_names is None:
        fetch_alias = target_var_names
    else:
        fetch_alias = fetch_alias_names.split(',')
    if len(feed_alias) != len(feed_var_dict.keys()) or len(fetch_alias) != len(
            target_var_names):
        raise ValueError(
            "please check the input --feed_alias_names and --fetch_alias_names, should be same size with feed_vars and fetch_vars"
        )
    for i, key in enumerate(feed_var_dict):
        feed_var = model_conf.FeedVar()
        feed_var.alias_name = feed_alias[i]
        feed_var.name = feed_var_dict[key].name
        feed_var.feed_type = var_type_conversion(feed_var_dict[key].dtype)

        feed_var.is_lod_tensor = feed_var_dict[
            key].lod_level >= 1 if feed_var_dict[
                key].lod_level is not None else False
        if feed_var.is_lod_tensor:
            feed_var.shape.extend([-1])
        else:
            tmp_shape = []
            for v in feed_var_dict[key].shape:
                if v >= 0:
                    tmp_shape.append(v)
            feed_var.shape.extend(tmp_shape)
        config.feed_var.extend([feed_var])

    for i, key in enumerate(target_var_names):
        fetch_var = model_conf.FetchVar()
        fetch_var.alias_name = fetch_alias[i]
        fetch_var.name = fetch_var_dict[key].name
        fetch_var.fetch_type = var_type_conversion(fetch_var_dict[key].dtype)

        fetch_var.is_lod_tensor = fetch_var_dict[key].lod_level >= 1
        #fetch_var.is_lod_tensor = 1
        if fetch_var.is_lod_tensor:
            fetch_var.shape.extend([-1])
        else:
            tmp_shape = []
            for v in fetch_var_dict[key].shape:
                if v >= 0:
                    tmp_shape.append(v)
            fetch_var.shape.extend(tmp_shape)
        config.fetch_var.extend([fetch_var])

    if show_proto:
        print(str(config))
        return
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
                               encrypt_conf=None,
                               show_proto=False,
                               feed_alias_names=None,
                               fetch_alias_names=None):
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
               inference_program, encryption, key_len, encrypt_conf,
               model_filename, params_filename, show_proto, feed_alias_names,
               fetch_alias_names)
    feed_names = feed_dict.keys()
    fetch_names = fetch_dict.keys()
    return feed_names, fetch_names
