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
import numpy as np
import requests
import json
import base64
import time
import sys

sys.path.append(
    os.path.join(os.path.abspath(os.path.dirname(__file__)), 'proto'))

#param 'type'(which is in feed_var or fetch_var) = 0 means dataType is int64
#param 'type'(which is in feed_var or fetch_var) = 1 means dataType is float32
#param 'type'(which is in feed_var or fetch_var) = 2 means dataType is int32
#param 'type'(which is in feed_var or fetch_var) = 5 means dataType is float16
#param 'type'(which is in feed_var or fetch_var) = 7 means dataType is uint8
#param 'type'(which is in feed_var or fetch_var) = 8 means dataType is int8
#param 'type'(which is in feed_var or fetch_var) = 20 means dataType is string(also called bytes in proto)
int64_type = 0
float32_type = 1
int32_type = 2
float16_type = 5
uint8_type = 7
int8_type = 8
bytes_type = 20
#int_type,float_type,string_type are the set of each subdivision classes.
int_type = set([int64_type, int32_type])
float_type = set([float32_type])
string_type = set([bytes_type, float16_type, uint8_type, int8_type])


class _NOPProfiler(object):
    def record(self, name):
        pass

    def print_profile(self):
        pass


class _TimeProfiler(object):
    def __init__(self):
        self.pid = os.getpid()
        self.print_head = 'PROFILE\tpid:{}\t'.format(self.pid)
        self.time_record = [self.print_head]

    def record(self, name):
        self.time_record.append('{}:{} '.format(
            name, int(round(time.time() * 1000000))))

    def print_profile(self):
        self.time_record.append('\n')
        sys.stderr.write(''.join(self.time_record))
        self.time_record = [self.print_head]


_is_profile = int(os.environ.get('FLAGS_profile_client', 0))
_Profiler = _TimeProfiler if _is_profile else _NOPProfiler


class SDKConfig(object):
    def __init__(self):
        self.sdk_desc = sdk.SDKConf()
        self.tag_list = []
        self.cluster_list = []
        self.variant_weight_list = []
        self.rpc_timeout_ms = 200000
        self.load_balance_strategy = "la"

    def add_server_variant(self, tag, cluster, variant_weight):
        self.tag_list.append(tag)
        self.cluster_list.append(cluster)
        self.variant_weight_list.append(variant_weight)

    def set_load_banlance_strategy(self, strategy):
        self.load_balance_strategy = strategy

    def gen_desc(self, rpc_timeout_ms):
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
        self.sdk_desc.default_variant_conf.connection_conf.rpc_timeout_ms = rpc_timeout_ms
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
        self.feed_shapes_ = {}
        self.feed_types_ = {}
        self.feed_names_to_idx_ = {}
        self.pid = os.getpid()
        self.predictor_sdk_ = None
        self.producers = []
        self.consumer = None
        self.profile_ = _Profiler()
        self.all_numpy_input = True
        self.has_numpy_input = False
        self.rpc_timeout_ms = 200000
        from .serving_client import PredictorRes
        self.predictorres_constructor = PredictorRes

    def load_client_config(self, model_config_path_list):
        if isinstance(model_config_path_list, str):
            model_config_path_list = [model_config_path_list]
        elif isinstance(model_config_path_list, list):
            pass

        file_path_list = []
        for single_model_config in model_config_path_list:
            if os.path.isdir(single_model_config):
                file_path_list.append("{}/serving_client_conf.prototxt".format(
                    single_model_config))
            elif os.path.isfile(single_model_config):
                file_path_list.append(single_model_config)
        from .serving_client import PredictorClient
        model_conf = m_config.GeneralModelConfig()
        f = open(file_path_list[0], 'r')
        model_conf = google.protobuf.text_format.Merge(
            str(f.read()), model_conf)

        # load configuraion here
        # get feed vars, fetch vars
        # get feed shapes, feed types
        # map feed names to index
        self.client_handle_ = PredictorClient()
        self.client_handle_.init(file_path_list)
        if "FLAGS_max_body_size" not in os.environ:
            os.environ["FLAGS_max_body_size"] = str(512 * 1024 * 1024)
        read_env_flags = ["profile_client", "profile_server", "max_body_size"]
        self.client_handle_.init_gflags([sys.argv[
            0]] + ["--tryfromenv=" + ",".join(read_env_flags)])
        self.feed_names_ = [var.alias_name for var in model_conf.feed_var]
        self.feed_names_to_idx_ = {}  #this is not useful
        self.lod_tensor_set = set()
        self.feed_tensor_len = {}  #this is only used for shape check
        self.key = None

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
        if len(file_path_list) > 1:
            model_conf = m_config.GeneralModelConfig()
            f = open(file_path_list[-1], 'r')
            model_conf = google.protobuf.text_format.Merge(
                str(f.read()), model_conf)
        self.fetch_names_ = [var.alias_name for var in model_conf.fetch_var]
        self.fetch_names_to_type_ = {}
        self.fetch_names_to_idx_ = {}
        for i, var in enumerate(model_conf.fetch_var):
            self.fetch_names_to_idx_[var.alias_name] = i
            self.fetch_names_to_type_[var.alias_name] = var.fetch_type
            if var.is_lod_tensor:
                self.lod_tensor_set.add(var.alias_name)
        return

    def add_variant(self, tag, cluster, variant_weight):
        if self.predictor_sdk_ is None:
            self.predictor_sdk_ = SDKConfig()
        self.predictor_sdk_.add_server_variant(tag, cluster,
                                               str(variant_weight))

    def set_rpc_timeout_ms(self, rpc_timeout):
        if not isinstance(rpc_timeout, int):
            raise ValueError("rpc_timeout must be int type.")
        else:
            self.rpc_timeout_ms = rpc_timeout

    def use_key(self, key_filename):
        with open(key_filename, "rb") as f:
            self.key = f.read()

    def get_serving_port(self, endpoints):
        if self.key is not None:
            req = json.dumps({"key": base64.b64encode(self.key).decode()})
        else:
            req = json.dumps({})
        r = requests.post("http://" + endpoints[0], req)
        result = r.json()
        print(result)
        if "endpoint_list" not in result:
            raise ValueError("server not ready")
        else:
            endpoints = [
                endpoints[0].split(":")[0] + ":" +
                str(result["endpoint_list"][0])
            ]
            return endpoints

    def connect(self, endpoints=None, encryption=False):
        # check whether current endpoint is available
        # init from client config
        # create predictor here
        if endpoints is None:
            if self.predictor_sdk_ is None:
                raise ValueError(
                    "You must set the endpoints parameter or use add_variant function to create a variant."
                )
        else:
            if encryption:
                endpoints = self.get_serving_port(endpoints)
            if self.predictor_sdk_ is None:
                self.add_variant('default_tag_{}'.format(id(self)), endpoints,
                                 100)
            else:
                print(
                    "parameter endpoints({}) will not take effect, because you use the add_variant function.".
                    format(endpoints))
        sdk_desc = self.predictor_sdk_.gen_desc(self.rpc_timeout_ms)
        self.client_handle_.create_predictor_by_desc(sdk_desc.SerializeToString(
        ))

    def get_feed_names(self):
        return self.feed_names_

    def get_fetch_names(self):
        return self.fetch_names_

    def shape_check(self, feed, key):
        if key in self.lod_tensor_set:
            return
        if isinstance(feed[key],
                      list) and len(feed[key]) != self.feed_tensor_len[key]:
            raise ValueError("The shape of feed tensor {} not match.".format(
                key))
        if type(feed[key]).__module__ == np.__name__ and np.size(feed[
                key]) != self.feed_tensor_len[key]:
            #raise SystemExit("The shape of feed tensor {} not match.".format(
            #    key))
            pass

    def predict(self,
                feed=None,
                fetch=None,
                batch=False,
                need_variant_tag=False,
                log_id=0):
        self.profile_.record('py_prepro_0')

        # fetch 可以为空，此时会取所有的输出结果
        if feed is None:
            raise ValueError("You should specify feed for prediction")

        fetch_list = []
        if isinstance(fetch, str):
            fetch_list = [fetch]
        elif isinstance(fetch, list):
            fetch_list = fetch
        # fetch 可以为空，此时会取所有的输出结果
        elif fetch == None:
            pass
        else:
            raise ValueError("Fetch only accepts string or list of string")

        feed_batch = []
        if isinstance(feed, dict):
            feed_batch.append(feed)
        elif isinstance(feed, list):
            # feed = [dict]
            if len(feed) == 1 and isinstance(feed[0], dict):
                feed_batch = feed
            else:
                # if input is a list and the number of feed_var is 1.
                # create a temp_dict { key = feed_var_name, value = list}
                # put the temp_dict into the feed_batch.
                if len(self.feed_names_) != 1:
                    raise ValueError(
                        "input is a list, but we got 0 or 2+ feed_var, don`t know how to divide the feed list"
                    )
                temp_dict = {}
                temp_dict[self.feed_names_[0]] = feed
                feed_batch.append(temp_dict)
        else:
            raise ValueError("Feed only accepts dict and list of dict")

        # batch_size must be 1, cause batch is already in Tensor.
        if len(feed_batch) != 1:
            raise ValueError("len of feed_batch can only be 1.")

        int32_slot = []
        int32_feed_names = []
        int32_shape = []
        int32_lod_slot_batch = []

        int64_slot = []
        int64_feed_names = []
        int64_shape = []
        int64_lod_slot_batch = []

        float_slot = []
        float_feed_names = []
        float_lod_slot_batch = []
        float_shape = []

        string_slot = []
        string_feed_names = []
        string_lod_slot_batch = []
        string_shape = []
        fetch_names = []

        for key in fetch_list:
            if key in self.fetch_names_:
                fetch_names.append(key)

        feed_dict = feed_batch[0]
        for key in feed_dict:
            if ".lod" not in key and key not in self.feed_names_:
                raise ValueError("Wrong feed name: {}.".format(key))
            if ".lod" in key:
                continue

            self.shape_check(feed_dict, key)
            if self.feed_types_[key] in int_type:
                shape_lst = []
                if batch == False:
                    feed_dict[key] = np.expand_dims(feed_dict[key], 0).repeat(
                        1, axis=0)
                # verify different input int_type
                if(self.feed_types_[key] == int64_type):
                    int64_feed_names.append(key)
                    if isinstance(feed_dict[key], np.ndarray):
                        shape_lst.extend(list(feed_dict[key].shape))
                        int64_shape.append(shape_lst)
                        self.has_numpy_input = True
                    else:
                        int64_shape.append(self.feed_shapes_[key])
                        self.all_numpy_input = False
                    if "{}.lod".format(key) in feed_dict:
                        int64_lod_slot_batch.append(feed_dict["{}.lod".format(key)])
                    else:
                        int64_lod_slot_batch.append([])
                    int64_slot.append(np.ascontiguousarray(feed_dict[key]))
                else:
                    int32_feed_names.append(key)
                    if isinstance(feed_dict[key], np.ndarray):
                        shape_lst.extend(list(feed_dict[key].shape))
                        int32_shape.append(shape_lst)
                        self.has_numpy_input = True
                    else:
                        int32_shape.append(self.feed_shapes_[key])
                        self.all_numpy_input = False
                    if "{}.lod".format(key) in feed_dict:
                        int32_lod_slot_batch.append(feed_dict["{}.lod".format(key)])
                    else:
                        int32_lod_slot_batch.append([])
                    int32_slot.append(np.ascontiguousarray(feed_dict[key]))

            elif self.feed_types_[key] in float_type:
                float_feed_names.append(key)
                shape_lst = []
                if batch == False:
                    feed_dict[key] = np.expand_dims(feed_dict[key], 0).repeat(
                        1, axis=0)
                if isinstance(feed_dict[key], np.ndarray):
                    shape_lst.extend(list(feed_dict[key].shape))
                    float_shape.append(shape_lst)
                else:
                    float_shape.append(self.feed_shapes_[key])
                if "{}.lod".format(key) in feed_dict:
                    float_lod_slot_batch.append(feed_dict["{}.lod".format(key)])
                else:
                    float_lod_slot_batch.append([])

                if isinstance(feed_dict[key], np.ndarray):
                    float_slot.append(np.ascontiguousarray(feed_dict[key]))
                    self.has_numpy_input = True
                else:
                    float_slot.append(np.ascontiguousarray(feed_dict[key]))
                    self.all_numpy_input = False
            #if input is string, feed is not numpy.
            elif self.feed_types_[key] in string_type:
                string_feed_names.append(key)
                string_shape.append(self.feed_shapes_[key])
                if "{}.lod".format(key) in feed_dict:
                    string_lod_slot_batch.append(feed_dict["{}.lod".format(
                        key)])
                else:
                    string_lod_slot_batch.append([])
                if type(feed_dict[key]) is np.ndarray:
                    string_slot.append(feed_dict[key].tostring())
                else:
                    string_slot.append(feed_dict[key])
                self.has_numpy_input = True

        self.profile_.record('py_prepro_1')
        self.profile_.record('py_client_infer_0')

        result_batch_handle = self.predictorres_constructor()
        if self.all_numpy_input:
            res = self.client_handle_.numpy_predict(
                float_slot, float_feed_names, float_shape, float_lod_slot_batch,
                int32_slot, int32_feed_names, int32_shape, int32_lod_slot_batch,
                int64_slot, int64_feed_names, int64_shape, int64_lod_slot_batch,
                string_slot, string_feed_names, string_shape,
                string_lod_slot_batch, fetch_names, result_batch_handle,
                self.pid, log_id)
        elif self.has_numpy_input == False:
            raise ValueError(
                "Please make sure all of your inputs are numpy array")
        else:
            raise ValueError(
                "Please make sure the inputs are all in list type or all in numpy.array type"
            )

        self.profile_.record('py_client_infer_1')
        self.profile_.record('py_postpro_0')

        if res == -1:
            return None

        multi_result_map = []
        model_engine_names = result_batch_handle.get_engine_names()
        for mi, engine_name in enumerate(model_engine_names):
            result_map = {}
            # fetch 为空，则会取所有的输出结果
            if len(fetch_names) == 0:
                fetch_names = result_batch_handle.get_tensor_alias_names(mi)
            # result map needs to be a numpy array
            for i, name in enumerate(fetch_names):
                if self.fetch_names_to_type_[name] == int64_type:
                    # result_map[name] will be py::array(numpy array)
                    result_map[name] = result_batch_handle.get_int64_by_name(
                        mi, name)
                    shape = result_batch_handle.get_shape(mi, name)
                    if result_map[name].size == 0:
                        raise ValueError(
                            "Failed to fetch, maybe the type of [{}]"
                            " is wrong, please check the model file".format(
                                name))
                    result_map[name].shape = shape
                    if name in self.lod_tensor_set:
                        tmp_lod = result_batch_handle.get_lod(mi, name)
                        if np.size(tmp_lod) > 0:
                            result_map["{}.lod".format(name)] = tmp_lod
                elif self.fetch_names_to_type_[name] == float32_type:
                    result_map[name] = result_batch_handle.get_float_by_name(
                        mi, name)
                    if result_map[name].size == 0:
                        raise ValueError(
                            "Failed to fetch, maybe the type of [{}]"
                            " is wrong, please check the model file".format(
                                name))
                    shape = result_batch_handle.get_shape(mi, name)
                    result_map[name].shape = shape
                    if name in self.lod_tensor_set:
                        tmp_lod = result_batch_handle.get_lod(mi, name)
                        if np.size(tmp_lod) > 0:
                            result_map["{}.lod".format(name)] = tmp_lod
                elif self.fetch_names_to_type_[name] == int32_type:
                    # result_map[name] will be py::array(numpy array)
                    result_map[name] = result_batch_handle.get_int32_by_name(
                        mi, name)
                    if result_map[name].size == 0:
                        raise ValueError(
                            "Failed to fetch, maybe the type of [{}]"
                            " is wrong, please check the model file".format(
                                name))
                    shape = result_batch_handle.get_shape(mi, name)
                    result_map[name].shape = shape
                    if name in self.lod_tensor_set:
                        tmp_lod = result_batch_handle.get_lod(mi, name)
                        if np.size(tmp_lod) > 0:
                            result_map["{}.lod".format(name)] = tmp_lod
                elif self.fetch_names_to_type_[name] == uint8_type:
                    # result_map[name] will be py::array(numpy array)
                    tmp_str = result_batch_handle.get_string_by_name(
                        mi, name)
                    result_map[name] = np.fromstring(tmp_str, dtype = np.uint8)
                    if result_map[name].size == 0:
                        raise ValueError(
                            "Failed to fetch, maybe the type of [{}]"
                            " is wrong, please check the model file".format(
                                name))
                    shape = result_batch_handle.get_shape(mi, name)
                    result_map[name].shape = shape
                    if name in self.lod_tensor_set:
                        tmp_lod = result_batch_handle.get_lod(mi, name)
                        if np.size(tmp_lod) > 0:
                            result_map["{}.lod".format(name)] = tmp_lod
                elif self.fetch_names_to_type_[name] == int8_type:
                    # result_map[name] will be py::array(numpy array)
                    tmp_str = result_batch_handle.get_string_by_name(
                        mi, name)
                    result_map[name] = np.fromstring(tmp_str, dtype = np.int8)
                    if result_map[name].size == 0:
                        raise ValueError(
                            "Failed to fetch, maybe the type of [{}]"
                            " is wrong, please check the model file".format(
                                name))
                    shape = result_batch_handle.get_shape(mi, name)
                    result_map[name].shape = shape
                    if name in self.lod_tensor_set:
                        tmp_lod = result_batch_handle.get_lod(mi, name)
                        if np.size(tmp_lod) > 0:
                            result_map["{}.lod".format(name)] = tmp_lod
                elif self.fetch_names_to_type_[name] == float16_type:
                    # result_map[name] will be py::array(numpy array)
                    tmp_str = result_batch_handle.get_string_by_name(
                        mi, name)
                    result_map[name] = np.fromstring(tmp_str, dtype = np.float16)
                    if result_map[name].size == 0:
                        raise ValueError(
                            "Failed to fetch, maybe the type of [{}]"
                            " is wrong, please check the model file".format(
                                name))
                    shape = result_batch_handle.get_shape(mi, name)
                    result_map[name].shape = shape
                    if name in self.lod_tensor_set:
                        tmp_lod = result_batch_handle.get_lod(mi, name)
                        if np.size(tmp_lod) > 0:
                            result_map["{}.lod".format(name)] = tmp_lod
            multi_result_map.append(result_map)
        ret = None
        if len(model_engine_names) == 1:
            # If only one model result is returned, the format of ret is result_map
            ret = multi_result_map[0]
        else:
            # If multiple model results are returned, the format of ret is {name: result_map}
            ret = {
                engine_name: multi_result_map[mi]
                for mi, engine_name in enumerate(model_engine_names)
            }

        self.profile_.record('py_postpro_1')
        self.profile_.print_profile()

        # When using the A/B test, the tag of variant needs to be returned
        return ret if not need_variant_tag else [
            ret, result_batch_handle.variant_tag()
        ]

    def release(self):
        self.client_handle_.destroy_predictor()
        self.client_handle_ = None
