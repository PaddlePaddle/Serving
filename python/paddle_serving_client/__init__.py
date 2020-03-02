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

import paddle_serving_client
import os
from .proto import sdk_configure_pb2 as sdk
from .proto import general_model_config_pb2 as m_config
import google.protobuf.text_format
import time
import sys

int_type = 0
float_type = 1


class SDKConfig(object):
    def __init__(self):
        self.sdk_desc = sdk.SDKConf()
        self.endpoints = []

    def set_server_endpoints(self, endpoints):
        self.endpoints = endpoints

    def gen_desc(self):
        predictor_desc = sdk.Predictor()
        predictor_desc.name = "general_model"
        predictor_desc.service_name = \
            "baidu.paddle_serving.predictor.general_model.GeneralModelService"
        predictor_desc.endpoint_router = "WeightedRandomRender"
        predictor_desc.weighted_random_render_conf.variant_weight_list = "100"

        variant_desc = sdk.VariantConf()
        variant_desc.tag = "var1"
        variant_desc.naming_conf.cluster = "list://{}".format(":".join(
            self.endpoints))

        predictor_desc.variants.extend([variant_desc])

        self.sdk_desc.predictors.extend([predictor_desc])
        self.sdk_desc.default_variant_conf.tag = "default"
        self.sdk_desc.default_variant_conf.connection_conf.connect_timeout_ms = 2000
        self.sdk_desc.default_variant_conf.connection_conf.rpc_timeout_ms = 20000
        self.sdk_desc.default_variant_conf.connection_conf.connect_retry_count = 2
        self.sdk_desc.default_variant_conf.connection_conf.max_connection_per_host = 100
        self.sdk_desc.default_variant_conf.connection_conf.hedge_request_timeout_ms = -1
        self.sdk_desc.default_variant_conf.connection_conf.hedge_fetch_retry_count = 2
        self.sdk_desc.default_variant_conf.connection_conf.connection_type = "pooled"

        self.sdk_desc.default_variant_conf.naming_conf.cluster_filter_strategy = "Default"
        self.sdk_desc.default_variant_conf.naming_conf.load_balance_strategy = "la"

        self.sdk_desc.default_variant_conf.rpc_parameter.compress_type = 0
        self.sdk_desc.default_variant_conf.rpc_parameter.package_size = 20
        self.sdk_desc.default_variant_conf.rpc_parameter.protocol = "baidu_std"
        self.sdk_desc.default_variant_conf.rpc_parameter.max_channel_per_request = 3

        return self.sdk_desc


class Client(object):
    def __init__(self):
        self.feed_names_ = []
        self.fetch_names_ = []
        self.client_handle_ = None
        self.result_handle_ = None
        self.feed_shapes_ = []
        self.feed_types_ = {}
        self.feed_names_to_idx_ = {}
        self.rpath()

    def rpath(self):
        lib_path = os.path.dirname(paddle_serving_client.__file__)
        client_path = os.path.join(lib_path, 'serving_client.so')
        lib_path = os.path.join(lib_path, 'lib')
        os.popen('patchelf --set-rpath {} {}'.format(lib_path, client_path))


    def load_client_config(self, path):
        from .serving_client import PredictorClient
        from .serving_client import PredictorRes
        model_conf = m_config.GeneralModelConfig()
        f = open(path, 'r')
        model_conf = google.protobuf.text_format.Merge(
            str(f.read()), model_conf)

        # load configuraion here
        # get feed vars, fetch vars
        # get feed shapes, feed types
        # map feed names to index
        self.result_handle_ = PredictorRes()
        self.client_handle_ = PredictorClient()
        self.client_handle_.init(path)
        read_env_flags = ["profile_client", "profile_server"]
        self.client_handle_.init_gflags([sys.argv[
            0]] + ["--tryfromenv=" + ",".join(read_env_flags)])
        self.feed_names_ = [var.alias_name for var in model_conf.feed_var]
        self.fetch_names_ = [var.alias_name for var in model_conf.fetch_var]
        self.feed_shapes_ = [var.shape for var in model_conf.feed_var]
        self.feed_names_to_idx_ = {}
        self.fetch_names_to_type_ = {}
        self.fetch_names_to_idx_ = {}
        for i, var in enumerate(model_conf.feed_var):
            self.feed_names_to_idx_[var.alias_name] = i
            self.feed_types_[var.alias_name] = var.feed_type

        for i, var in enumerate(model_conf.fetch_var):
            self.fetch_names_to_idx_[var.alias_name] = i
            self.fetch_names_to_type_[var.alias_name] = var.fetch_type

        return

    def connect(self, endpoints):
        # check whether current endpoint is available
        # init from client config
        # create predictor here
        predictor_sdk = SDKConfig()
        predictor_sdk.set_server_endpoints(endpoints)
        sdk_desc = predictor_sdk.gen_desc()
        print(sdk_desc)
        self.client_handle_.create_predictor_by_desc(
            sdk_desc.SerializeToString())
        

    def get_feed_names(self):
        return self.feed_names_

    def get_fetch_names(self):
        return self.fetch_names_

    def predict(self, feed={}, fetch=[]):
        int_slot = []
        float_slot = []
        int_feed_names = []
        float_feed_names = []
        fetch_names = []
        for key in feed:
            if key not in self.feed_names_:
                continue
            if self.feed_types_[key] == int_type:
                int_feed_names.append(key)
                int_slot.append(feed[key])
            elif self.feed_types_[key] == float_type:
                float_feed_names.append(key)
                float_slot.append(feed[key])

        for key in fetch:
            if key in self.fetch_names_:
                fetch_names.append(key)

        ret = self.client_handle_.predict(
            float_slot, float_feed_names, int_slot,
            int_feed_names, fetch_names, self.result_handle_)

        result_map = {}
        for i, name in enumerate(fetch_names):
            if self.fetch_names_to_type_[name] == int_type:
                result_map[name] = self.result_handle_.get_int64_by_name(name)[0]
            elif self.fetch_names_to_type_[name] == float_type:
                result_map[name] = self.result_handle_.get_float_by_name(name)[0]

        return result_map

    def batch_predict(self, feed_batch=[], fetch=[]):
        int_slot_batch = []
        float_slot_batch = []
        int_feed_names = []
        float_feed_names = []
        fetch_names = []
        counter = 0
        for feed in feed_batch:
            int_slot = []
            float_slot = []
            for key in feed:
                if key not in self.feed_names_:
                    continue
                if self.feed_types_[key] == int_type:
                    if counter == 0:
                        int_feed_names.append(key)
                    int_slot.append(feed[key])
                elif self.feed_types_[key] == float_type:
                    if counter == 0:
                        float_feed_names.append(key)
                    float_slot.append(feed[key])
            counter += 1
            int_slot_batch.append(int_slot)
            float_slot_batch.append(float_slot)

        for key in fetch:
            if key in self.fetch_names_:
                fetch_names.append(key)

        result_batch = self.client_handle_.batch_predict(
            float_slot_batch, float_feed_names, int_slot_batch, int_feed_names,
            fetch_names)

        result_map_batch = []
        for result in result_batch:
            result_map = {}
            for i, name in enumerate(fetch_names):
                result_map[name] = result[i]
            result_map_batch.append(result_map)

        return result_map_batch

    def release(self):
        self.client_handle_.destroy_predictor()
        self.client_handle_ = None
