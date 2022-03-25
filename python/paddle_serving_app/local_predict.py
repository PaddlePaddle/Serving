# -*- coding: utf-8 -*-
"""
# Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
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
"""

import os
import google.protobuf.text_format
import numpy as np
import argparse
from .proto import general_model_config_pb2 as m_config
import paddle.inference as paddle_infer
import logging
import glob
from paddle_serving_server.pipeline.error_catch import ErrorCatch, CustomException, CustomExceptionCode, ParamChecker, ParamVerify
check_dynamic_shape_info=ParamVerify.check_dynamic_shape_info

logging.basicConfig(format="%(asctime)s - %(levelname)s - %(message)s")
logger = logging.getLogger("LocalPredictor")
logger.setLevel(logging.INFO)
from paddle_serving_server.util import kill_stop_process_by_pid

precision_map = {
    'int8': paddle_infer.PrecisionType.Int8,
    'fp32': paddle_infer.PrecisionType.Float32,
    'fp16': paddle_infer.PrecisionType.Half,
    'bf16': 'bf16',
}


class LocalPredictor(object):
    """
    Prediction in the current process of the local environment, in process
    call, Compared with RPC/HTTP, LocalPredictor has better performance, 
    because of no network and packaging load.
    """

    def __init__(self):
        self.feed_names_ = []
        self.fetch_names_ = []
        self.feed_types_ = {}
        self.fetch_types_ = {}
        self.feed_shapes_ = {}
        self.feed_names_to_idx_ = {}
        self.fetch_names_to_idx_ = {}
        self.fetch_names_to_type_ = {}

    def search_suffix_files(self, model_path, target_suffix):
        """
        Find all files with the suffix xxx in the specified directory.

        Args:
            model_path: model directory, not None.
            target_suffix: filenames with target suffix, not None. e.g: *.pdmodel

        Returns:
            file_list, None, [] or [path, ] . 
        """
        if model_path is None or target_suffix is None:
            return None

        file_list = glob.glob(os.path.join(model_path, target_suffix))
        return file_list

    def load_model_config(self,
                          model_path,
                          use_gpu=False,
                          gpu_id=0,
                          use_profile=False,
                          thread_num=1,
                          mem_optim=True,
                          ir_optim=False,
                          use_trt=False,
                          use_lite=False,
                          use_xpu=False,
                          precision="fp32",
                          use_mkldnn=False,
                          mkldnn_cache_capacity=0,
                          mkldnn_op_list=None,
                          mkldnn_bf16_op_list=None,
                          use_feed_fetch_ops=False,
                          use_ascend_cl=False,
                          min_subgraph_size=3,
                          dynamic_shape_info={},
                          use_calib=False):
        """
        Load model configs and create the paddle predictor by Paddle Inference API.
   
        Args:
            model_path: model config path.
            use_gpu: calculating with gpu, False default.
            gpu_id: gpu id, 0 default.
            use_profile: use predictor profiles, False default.
            thread_num: thread nums of cpu math library, default 1. 
            mem_optim: memory optimization, True default.
            ir_optim: open calculation chart optimization, False default.
            use_trt: use nvidia TensorRT optimization, False default
            use_lite: use Paddle-Lite Engint, False default
            ir_optim: open calculation chart optimization, False default.
            use_trt: use nvidia TensorRT optimization, False default
            use_lite: use Paddle-Lite Engint, False default
            use_xpu: run predict on Baidu Kunlun, False default
            precision: precision mode, "fp32" default
            use_mkldnn: use MKLDNN, False default.
            mkldnn_cache_capacity: cache capacity for input shapes, 0 default.
            mkldnn_op_list: op list accelerated using MKLDNN, None default.
            mkldnn_bf16_op_list: op list accelerated using MKLDNN bf16, None default.
            use_feed_fetch_ops: use feed/fetch ops, False default.
            use_ascend_cl: run predict on Huawei Ascend, False default
            min_subgraph_size: the minimal subgraph size for opening tensorrt to optimize, 3 default
            dynamic_shape_info: dict including min_input_shape，max_input_shape, opt_input_shape, {} default 
            use_calib: use TensorRT calibration, False default
        """
        gpu_id = int(gpu_id)
        client_config = "{}/serving_server_conf.prototxt".format(model_path)
        model_conf = m_config.GeneralModelConfig()
        f = open(client_config, 'r')
        model_conf = google.protobuf.text_format.Merge(
            str(f.read()), model_conf)

        # Init paddle_infer config
        # Paddle's model files and parameter files have multiple naming rules:
        #   1) __model__, __params__
        #   2) *.pdmodel, *.pdiparams
        #   3) __model__, conv2d_1.w_0, conv2d_2.w_0, fc_1.w_0, conv2d_1.b_0, ... 
        pdmodel_file_list = self.search_suffix_files(model_path, "*.pdmodel")
        pdiparams_file_list = self.search_suffix_files(model_path,
                                                       "*.pdiparams")
        if os.path.exists(os.path.join(model_path, "__params__")):
            # case 1) initializing
            config = paddle_infer.Config(
                os.path.join(model_path, "__model__"),
                os.path.join(model_path, "__params__"))
        elif pdmodel_file_list and len(
                pdmodel_file_list) > 0 and pdiparams_file_list and len(
                    pdiparams_file_list) > 0:
            # case 2) initializing
            logger.info("pdmodel_file_list:{}, pdiparams_file_list:{}".format(
                pdmodel_file_list, pdiparams_file_list))
            config = paddle_infer.Config(pdmodel_file_list[0],
                                         pdiparams_file_list[0])
        else:
            # case 3) initializing.
            config = paddle_infer.Config(model_path)

        logger.info(
            "LocalPredictor load_model_config params: model_path:{}, use_gpu:{}, "
            "gpu_id:{}, use_profile:{}, thread_num:{}, mem_optim:{}, ir_optim:{}, "
            "use_trt:{}, use_lite:{}, use_xpu:{}, precision:{}, use_calib:{}, "
            "use_mkldnn:{}, mkldnn_cache_capacity:{}, mkldnn_op_list:{}, "
            "mkldnn_bf16_op_list:{}, use_feed_fetch_ops:{}, "
            "use_ascend_cl:{}, min_subgraph_size:{}, dynamic_shape_info:{}".
            format(model_path, use_gpu, gpu_id, use_profile, thread_num,
                   mem_optim, ir_optim, use_trt, use_lite, use_xpu, precision,
                   use_calib, use_mkldnn, mkldnn_cache_capacity, mkldnn_op_list,
                   mkldnn_bf16_op_list, use_feed_fetch_ops, use_ascend_cl,
                   min_subgraph_size, dynamic_shape_info))

        self.feed_names_ = [var.alias_name for var in model_conf.feed_var]
        self.fetch_names_ = [var.alias_name for var in model_conf.fetch_var]
        self.feed_names_to_idx_ = {}
        self.fetch_names_to_idx_ = {}

        for i, var in enumerate(model_conf.feed_var):
            self.feed_names_to_idx_[var.alias_name] = i
            self.feed_types_[var.alias_name] = var.feed_type
            self.feed_shapes_[var.alias_name] = var.shape

        for i, var in enumerate(model_conf.fetch_var):
            self.fetch_names_to_idx_[var.alias_name] = i
            self.fetch_types_[var.alias_name] = var.fetch_type
            self.fetch_names_to_type_[var.alias_name] = var.shape

        # set precision of inference.
        precision_type = paddle_infer.PrecisionType.Float32
        if precision is not None and precision.lower() in precision_map:
            precision_type = precision_map[precision.lower()]
        else:
            logger.warning("precision error!!! Please check precision:{}".
                           format(precision))
        # set profile
        if use_profile:
            config.enable_profile()
        # set memory optimization
        if mem_optim:
            config.enable_memory_optim()
        # set ir optimization, threads of cpu math library
        config.switch_ir_optim(ir_optim)
        # use feed & fetch ops
        config.switch_use_feed_fetch_ops(use_feed_fetch_ops)
        # pass optim
        config.delete_pass("conv_transpose_eltwiseadd_bn_fuse_pass")

        # set cpu & mkldnn
        config.set_cpu_math_library_num_threads(thread_num)
        if use_mkldnn:
            config.enable_mkldnn()
            if precision_type == "bf16":
                config.enable_mkldnn_bfloat16()
            if mkldnn_cache_capacity > 0:
                config.set_mkldnn_cache_capacity(mkldnn_cache_capacity)
            if mkldnn_op_list is not None:
                config.set_mkldnn_op(mkldnn_op_list)
        # set gpu
        if not use_gpu:
            config.disable_gpu()
        else:
            config.enable_use_gpu(100, gpu_id)
            if use_trt:
                config.enable_tensorrt_engine(
                    precision_mode=precision_type,
                    workspace_size=1 << 20,
                    max_batch_size=32,
                    min_subgraph_size=min_subgraph_size,
                    use_static=False,
                    use_calib_mode=use_calib)

                @ErrorCatch
                @ParamChecker
                def dynamic_shape_info_helper(dynamic_shape_info:lambda dynamic_shape_info: check_dynamic_shape_info(dynamic_shape_info)):
                    pass
                _, resp = dynamic_shape_info_helper(dynamic_shape_info)
                if resp.err_no != CustomExceptionCode.OK.value:
                    print("dynamic_shape_info configure error, it should contain [min_input_shape', 'max_input_shape', 'opt_input_shape' {}".format(resp.err_msg))
                    kill_stop_process_by_pid("kill", os.getpgid(os.getpid()))

                if len(dynamic_shape_info):
                    config.set_trt_dynamic_shape_info(
                        dynamic_shape_info['min_input_shape'],
                        dynamic_shape_info['max_input_shape'],
                        dynamic_shape_info['opt_input_shape'])
        # set lite
        if use_lite:
            config.enable_lite_engine(
                precision_mode=precision_type,
                zero_copy=True,
                passes_filter=[],
                ops_filter=[])
            config.switch_ir_optim(True)
        # set xpu
        if use_xpu:
            # 2MB l3 cache
            config.enable_xpu(8 * 1024 * 1024)
            config.set_xpu_device_id(gpu_id)
        # set ascend cl
        if use_ascend_cl:
            if use_lite:
                # for ascend 310
                nnadapter_device_names = "huawei_ascend_npu"
                nnadapter_context_properties = \
                    "HUAWEI_ASCEND_NPU_SELECTED_DEVICE_IDS={}".format(gpu_id)
                nnadapter_model_cache_dir = ""
                config.nnadapter() \
                .enable() \
                .set_device_names([nnadapter_device_names]) \
                .set_context_properties(nnadapter_context_properties) \
                .set_model_cache_dir(nnadapter_model_cache_dir)
            else:
                # for ascend 910
                config.enable_npu(gpu_id)
        # set cpu low precision
        if not use_gpu and not use_lite:
            if precision_type == paddle_infer.PrecisionType.Int8:
                logger.warning(
                    "PRECISION INT8 is not supported in CPU right now! Please use fp16 or bf16."
                )
                #config.enable_quantizer()
            if precision is not None and precision.lower() == "bf16":
                config.enable_mkldnn_bfloat16()
                if mkldnn_bf16_op_list is not None:
                    config.set_bfloat16_op(mkldnn_bf16_op_list)

        @ErrorCatch
        def create_predictor_check(config):
            predictor = paddle_infer.create_predictor(config)
            return predictor
        predictor, resp = create_predictor_check(config)
        if resp.err_no != CustomExceptionCode.OK.value:
            logger.critical(
                "failed to create predictor: {}".format(resp.err_msg),
                exc_info=False)
            print("failed to create predictor: {}".format(resp.err_msg))
            kill_stop_process_by_pid("kill", os.getpgid(os.getpid()))
        self.predictor = predictor

    def predict(self, feed=None, fetch=None, batch=False, log_id=0):
        """
        Run model inference by Paddle Inference API.

        Args:
            feed: feed var list, None is not allowed.
            fetch: fetch var list, None allowed. when it is None, all fetch 
                   vars are returned. Otherwise, return fetch specified result.
            batch: batch data or not, False default.If batch is False, a new
                   dimension is added to header of the shape[np.newaxis].
            log_id: for logging

        Returns:
            fetch_map: dict 
        """
        if feed is None:
            raise ValueError("You should specify feed vars for prediction.\
                log_id:{}".format(log_id))

        feed_batch = []
        if isinstance(feed, dict):
            feed_batch.append(feed)
        elif isinstance(feed, list):
            feed_batch = feed
        else:
            raise ValueError("Feed only accepts dict and list of dict.\
                log_id:{}".format(log_id))

        fetch_list = []
        if fetch is not None:
            if isinstance(fetch, str):
                fetch_list = [fetch]
            elif isinstance(fetch, list):
                fetch_list = fetch

        # Filter invalid fetch names
        fetch_names = []
        for key in fetch_list:
            if key in self.fetch_names_:
                fetch_names.append(key)

        # Assemble the input data of paddle predictor, and filter invalid inputs. 
        input_names = self.predictor.get_input_names()
        for name in input_names:
            if isinstance(feed[name], list) and not isinstance(feed[name][0],
                                                               str):
                feed[name] = np.array(feed[name]).reshape(self.feed_shapes_[
                    name])
            if self.feed_types_[name] == 0:
                feed[name] = feed[name].astype("int64")
            elif self.feed_types_[name] == 1:
                feed[name] = feed[name].astype("float32")
            elif self.feed_types_[name] == 2:
                feed[name] = feed[name].astype("int32")
            elif self.feed_types_[name] == 3:
                feed[name] = feed[name].astype("float64")
            elif self.feed_types_[name] == 4:
                feed[name] = feed[name].astype("int16")
            elif self.feed_types_[name] == 5:
                feed[name] = feed[name].astype("float16")
            elif self.feed_types_[name] == 6:
                feed[name] = feed[name].astype("uint16")
            elif self.feed_types_[name] == 7:
                feed[name] = feed[name].astype("uint8")
            elif self.feed_types_[name] == 8:
                feed[name] = feed[name].astype("int8")
            elif self.feed_types_[name] == 9:
                feed[name] = feed[name].astype("bool")
            elif self.feed_types_[name] == 10:
                feed[name] = feed[name].astype("complex64")
            elif self.feed_types_[name] == 11:
                feed[name] = feed[name].astype("complex128")
            elif isinstance(feed[name], list) and isinstance(feed[name][0],
                                                             str):
                pass
            else:
                raise ValueError("local predictor receives wrong data type")

            input_tensor_handle = self.predictor.get_input_handle(name)
            if "{}.lod".format(name) in feed:
                input_tensor_handle.set_lod([feed["{}.lod".format(name)]])
            if batch == False:
                input_tensor_handle.copy_from_cpu(feed[name][np.newaxis, :])
            else:
                input_tensor_handle.copy_from_cpu(feed[name])

        # set output tensor handlers
        output_tensor_handles = []
        output_name_to_index_dict = {}
        output_names = self.predictor.get_output_names()
        for i, output_name in enumerate(output_names):
            output_tensor_handle = self.predictor.get_output_handle(output_name)
            output_tensor_handles.append(output_tensor_handle)
            output_name_to_index_dict[output_name] = i

        # Run inference 
        self.predictor.run()

        # Assemble output data of predict results
        outputs = []
        for output_tensor_handle in output_tensor_handles:
            output = output_tensor_handle.copy_to_cpu()
            outputs.append(output)
        outputs_len = len(outputs)

        # Copy fetch vars. If fetch is None, it will copy all results from output_tensor_handles. 
        # Otherwise, it will copy the fields specified from output_tensor_handles.
        fetch_map = {}
        if fetch is None:
            for i, name in enumerate(output_names):
                fetch_map[name] = outputs[i]
                if len(output_tensor_handles[i].lod()) > 0:
                    fetch_map[name + ".lod"] = np.array(output_tensor_handles[
                        i].lod()[0]).astype('int32')
        else:
            # Because the save_inference_model interface will increase the scale op 
            # in the network, the name of fetch_var is different from that in prototxt. 
            # Therefore, it is compatible with v0.6.x and the previous model save format,
            # and here is compatible with the results that do not match.
            fetch_match_num = 0
            for i, name in enumerate(fetch):
                output_index = output_name_to_index_dict.get(name)
                if output_index is None:
                    continue

                fetch_map[name] = outputs[output_index]
                fetch_match_num += 1
                if len(output_tensor_handles[output_index].lod()) > 0:
                    fetch_map[name + ".lod"] = np.array(output_tensor_handles[
                        output_index].lod()[0]).astype('int32')

            # Compatible with v0.6.x and lower versions model saving formats.
            if fetch_match_num == 0:
                logger.debug("fetch match num is 0. Retrain the model please!")
                for i, name in enumerate(fetch):
                    if i >= outputs_len:
                        break
                    fetch_map[name] = outputs[i]
                    if len(output_tensor_handles[i].lod()) > 0:
                        fetch_map[name + ".lod"] = np.array(
                            output_tensor_handles[i].lod()[0]).astype('int32')

        return fetch_map
