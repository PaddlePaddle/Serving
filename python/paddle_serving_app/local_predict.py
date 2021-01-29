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
import paddle.fluid as fluid
import paddle.inference as inference
from .proto import general_model_config_pb2 as m_config
from paddle.fluid.core import PaddleTensor
from paddle.fluid.core import AnalysisConfig
from paddle.fluid.core import create_paddle_predictor
import logging

logging.basicConfig(format="%(asctime)s - %(levelname)s - %(message)s")
logger = logging.getLogger("fluid")
logger.setLevel(logging.INFO)


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
                          use_feed_fetch_ops=False):
        """
        Load model config and set the engine config for the paddle predictor
   
        Args:
            model_path: model config path.
            use_gpu: calculating with gpu, False default.
            gpu_id: gpu id, 0 default.
            use_profile: use predictor profiles, False default.
            thread_num: thread nums, default 1. 
            mem_optim: memory optimization, True default.
            ir_optim: open calculation chart optimization, False default.
            use_trt: use nvidia TensorRT optimization, False default
            use_lite: use Paddle-Lite Engint, False default
            use_xpu: run predict on Baidu Kunlun, False default
            use_feed_fetch_ops: use feed/fetch ops, False default.
        """
        client_config = "{}/serving_server_conf.prototxt".format(model_path)
        model_conf = m_config.GeneralModelConfig()
        f = open(client_config, 'r')
        model_conf = google.protobuf.text_format.Merge(
            str(f.read()), model_conf)
        config = AnalysisConfig(model_path)
        logger.info("load_model_config params: model_path:{}, use_gpu:{},\
            gpu_id:{}, use_profile:{}, thread_num:{}, mem_optim:{}, ir_optim:{},\
            use_trt:{}, use_lite:{}, use_xpu: {}, use_feed_fetch_ops:{}".format(
            model_path, use_gpu, gpu_id, use_profile, thread_num, mem_optim,
            ir_optim, use_trt, use_lite, use_xpu, use_feed_fetch_ops))

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
            self.fetch_names_to_type_[var.alias_name] = var.fetch_type

        if use_profile:
            config.enable_profile()
        if mem_optim:
            config.enable_memory_optim()
        config.switch_ir_optim(ir_optim)
        config.set_cpu_math_library_num_threads(thread_num)
        config.switch_use_feed_fetch_ops(use_feed_fetch_ops)
        config.delete_pass("conv_transpose_eltwiseadd_bn_fuse_pass")

        if not use_gpu:
            config.disable_gpu()
        else:
            config.enable_use_gpu(100, gpu_id)
            if use_trt:
                config.enable_tensorrt_engine(
                    workspace_size=1 << 20,
                    max_batch_size=32,
                    min_subgraph_size=3,
                    use_static=False,
                    use_calib_mode=False)

        if use_lite:
            config.enable_lite_engine(
                precision_mode=inference.PrecisionType.Float32,
                zero_copy=True,
                passes_filter=[],
                ops_filter=[])

        if use_xpu:
            # 2MB l3 cache
            config.enable_xpu(8 * 1024 * 1024)

        self.predictor = create_paddle_predictor(config)

    def predict(self, feed=None, fetch=None, batch=False, log_id=0):
        """
        Predict locally

        Args:
            feed: feed var
            fetch: fetch var
            batch: batch data or not, False default.If batch is False, a new
                   dimension is added to header of the shape[np.newaxis].
            log_id: for logging

        Returns:
            fetch_map: dict 
        """
        if feed is None or fetch is None:
            raise ValueError("You should specify feed and fetch for prediction")
        fetch_list = []
        if isinstance(fetch, str):
            fetch_list = [fetch]
        elif isinstance(fetch, list):
            fetch_list = fetch
        else:
            raise ValueError("Fetch only accepts string and list of string")

        feed_batch = []
        if isinstance(feed, dict):
            feed_batch.append(feed)
        elif isinstance(feed, list):
            feed_batch = feed
        else:
            raise ValueError("Feed only accepts dict and list of dict")

        int_slot_batch = []
        float_slot_batch = []
        int_feed_names = []
        float_feed_names = []
        int_shape = []
        float_shape = []
        fetch_names = []
        counter = 0
        batch_size = len(feed_batch)

        for key in fetch_list:
            if key in self.fetch_names_:
                fetch_names.append(key)

        if len(fetch_names) == 0:
            raise ValueError(
                "Fetch names should not be empty or out of saved fetch list.")
            return {}

        input_names = self.predictor.get_input_names()
        for name in input_names:
            if isinstance(feed[name], list):
                feed[name] = np.array(feed[name]).reshape(self.feed_shapes_[
                    name])
            if self.feed_types_[name] == 0:
                feed[name] = feed[name].astype("int64")
            elif self.feed_types_[name] == 1:
                feed[name] = feed[name].astype("float32")
            elif self.feed_types_[name] == 2:
                feed[name] = feed[name].astype("int32")
            else:
                raise ValueError("local predictor receives wrong data type")
            input_tensor = self.predictor.get_input_tensor(name)
            if "{}.lod".format(name) in feed:
                input_tensor.set_lod([feed["{}.lod".format(name)]])
            if batch == False:
                input_tensor.copy_from_cpu(feed[name][np.newaxis, :])
            else:
                input_tensor.copy_from_cpu(feed[name])
        output_tensors = []
        output_names = self.predictor.get_output_names()
        for output_name in output_names:
            output_tensor = self.predictor.get_output_tensor(output_name)
            output_tensors.append(output_tensor)
        outputs = []
        self.predictor.zero_copy_run()
        for output_tensor in output_tensors:
            output = output_tensor.copy_to_cpu()
            outputs.append(output)
        fetch_map = {}
        for i, name in enumerate(fetch):
            fetch_map[name] = outputs[i]
            if len(output_tensors[i].lod()) > 0:
                fetch_map[name + ".lod"] = np.array(output_tensors[i].lod()[
                    0]).astype('int32')
        return fetch_map
