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
        self.tag_list = []
        self.cluster_list = []
        self.variant_weight_list = []

    def add_server_variant(self, tag, cluster, variant_weight):
        self.tag_list.append(tag)
        self.cluster_list.append(cluster)
        self.variant_weight_list.append(variant_weight)

    def gen_desc(self):
        predictor_desc = sdk.Predictor()
        predictor_desc.name = "general_model"
        predictor_desc.service_name = \
            "baidu.paddle_serving.predictor.general_model.GeneralModelService"
        predictor_desc.endpoint_router = "WeightedRandomRender"
        predictor_desc.weighted_random_render_conf.variant_weight_list = "|".join(
            self.variant_weight_list)

        for idx, tag in enumerate(self.tag_list):
            variant_desc = sdk.VariantConf()
            variant_desc.tag = tag
            variant_desc.naming_conf.cluster = "list://{}".format(",".join(
                self.cluster_list[idx]))
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
        self.feed_shapes_ = {}
        self.feed_types_ = {}
        self.feed_names_to_idx_ = {}
        self.rpath()
        self.pid = os.getpid()
        self.predictor_sdk_ = None
        self.producers = []
        self.consumer = None

    def rpath(self):
        lib_path = os.path.dirname(paddle_serving_client.__file__)
        client_path = os.path.join(lib_path, 'serving_client.so')
        lib_path = os.path.join(lib_path, 'lib')
        os.system('patchelf --set-rpath {} {}'.format(lib_path, client_path))

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
        self.feed_names_to_idx_ = {}
        self.fetch_names_to_type_ = {}
        self.fetch_names_to_idx_ = {}
        self.lod_tensor_set = set()
        self.feed_tensor_len = {}
        for i, var in enumerate(model_conf.feed_var):
            self.feed_names_to_idx_[var.alias_name] = i
            self.feed_types_[var.alias_name] = var.feed_type
            self.feed_shapes_[var.alias_name] = var.shape

            if var.is_lod_tensor:
                self.lod_tensor_set.add(var.alias_name)
            else:
                counter = 1
                for dim in self.feed_shapes_[var.alias_name]:
                    counter *= dim
                self.feed_tensor_len[var.alias_name] = counter

        for i, var in enumerate(model_conf.fetch_var):
            self.fetch_names_to_idx_[var.alias_name] = i
            self.fetch_names_to_type_[var.alias_name] = var.fetch_type

        return

    def add_variant(self, tag, cluster, variant_weight):
        if self.predictor_sdk_ is None:
            self.predictor_sdk_ = SDKConfig()
        self.predictor_sdk_.add_server_variant(tag, cluster,
                                               str(variant_weight))

    def connect(self, endpoints=None):
        # check whether current endpoint is available
        # init from client config
        # create predictor here
        if endpoints is None:
            if self.predictor_sdk_ is None:
                raise SystemExit(
                    "You must set the endpoints parameter or use add_variant function to create a variant."
                )
        else:
            if self.predictor_sdk_ is None:
                timestamp = time.time()
                self.add_variant('default_tag_{}'.format(timestamp), endpoints,
                                 100)
            else:
                print(
                    "parameter endpoints({}) will not take effect, because you use the add_variant function.".
                    format(endpoints))
        sdk_desc = self.predictor_sdk_.gen_desc()
        print(sdk_desc)
        self.client_handle_.create_predictor_by_desc(sdk_desc.SerializeToString(
        ))

    def get_feed_names(self):
        return self.feed_names_

    def get_fetch_names(self):
        return self.fetch_names_

    def shape_check(self, feed, key):
        seq_shape = 1
        if key in self.lod_tensor_set:
            return
        if len(feed[key]) != self.feed_tensor_len[key]:
            raise SystemExit("The shape of feed tensor {} not match.".format(
                key))

    def predict(self, feed=None, fetch=None, need_variant_tag=False):
        if feed is None or fetch is None:
            raise ValueError("You should specify feed and fetch for prediction")

        fetch_list = []
        if isinstance(fetch, str):
            fetch_list = [fetch]
        elif isinstance(fetch, list):
            fetch_list = fetch
        else:
            raise ValueError("fetch only accepts string and list of string")

        feed_batch = []
        if isinstance(feed, dict):
            feed_batch.append(feed)
        elif isinstance(feed, list):
            feed_batch = feed
        else:
            raise ValueError("feed only accepts dict and list of dict")

        int_slot_batch = []
        float_slot_batch = []
        int_feed_names = []
        float_feed_names = []
        fetch_names = []
        counter = 0
        batch_size = len(feed_batch)

        for key in fetch_list:
            if key in self.fetch_names_:
                fetch_names.append(key)

        if len(fetch_names) == 0:
            raise ValueError(
                "fetch names should not be empty or out of saved fetch list")
            return {}

        for i, feed_i in enumerate(feed_batch):
            int_slot = []
            float_slot = []
            for key in feed_i:
                if key not in self.feed_names_:
                    continue
                if self.feed_types_[key] == int_type:
                    if i == 0:
                        int_feed_names.append(key)
                    int_slot.append(feed_i[key])
                elif self.feed_types_[key] == float_type:
                    if i == 0:
                        float_feed_names.append(key)
                    float_slot.append(feed_i[key])
            int_slot_batch.append(int_slot)
            float_slot_batch.append(float_slot)

        result_batch = self.result_handle_
        res = self.client_handle_.batch_predict(
            float_slot_batch, float_feed_names, int_slot_batch, int_feed_names,
            fetch_names, result_batch, self.pid)

        if res == -1:
            return None

        result_map_batch = []
        result_map = {}
        for i, name in enumerate(fetch_names):
            if self.fetch_names_to_type_[name] == int_type:
                result_map[name] = result_batch.get_int64_by_name(name)
            elif self.fetch_names_to_type_[name] == float_type:
                result_map[name] = result_batch.get_float_by_name(name)
        for i in range(batch_size):
            single_result = {}
            for key in result_map:
                single_result[key] = result_map[key][i]
            result_map_batch.append(single_result)

        if batch_size == 1:
            return [result_map_batch[0], self.result_handle_.variant_tag()
                    ] if need_variant_tag else result_map_batch[0]
        else:
            return [result_map_batch, self.result_handle_.variant_tag()
                    ] if need_variant_tag else result_map_batch

    def release(self):
        self.client_handle_.destroy_predictor()
        self.client_handle_ = None
