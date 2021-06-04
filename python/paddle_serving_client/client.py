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

import grpc
from .proto import multi_lang_general_model_service_pb2
sys.path.append(
    os.path.join(os.path.abspath(os.path.dirname(__file__)), 'proto'))
from .proto import multi_lang_general_model_service_pb2_grpc

#param 'type'(which is in feed_var or fetch_var) = 0 means dataType is int64
#param 'type'(which is in feed_var or fetch_var) = 1 means dataType is float32
#param 'type'(which is in feed_var or fetch_var) = 2 means dataType is int32
#param 'type'(which is in feed_var or fetch_var) = 3 means dataType is string(also called bytes in proto)
int64_type = 0
float32_type = 1
int32_type = 2
bytes_type = 3
#int_type,float_type,string_type are the set of each subdivision classes.
int_type = set([int64_type, int32_type])
float_type = set([float32_type])
string_type = set([bytes_type])


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
        self.rpc_timeout_ms = 20000
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
        self.rpc_timeout_ms = 20000
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
        int_feed_names = []
        int_shape = []
        int_lod_slot_batch = []
        float_slot_batch = []
        float_feed_names = []
        float_lod_slot_batch = []
        float_shape = []
        string_slot_batch = []
        string_feed_names = []
        string_lod_slot_batch = []
        string_shape = []

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

        for i, feed_i in enumerate(feed_batch):
            int_slot = []
            int_lod_slot = []
            float_slot = []
            float_lod_slot = []
            string_slot = []
            string_lod_slot = []
            for key in feed_i:
                if ".lod" not in key and key not in self.feed_names_:
                    raise ValueError("Wrong feed name: {}.".format(key))
                if ".lod" in key:
                    continue
                #if not isinstance(feed_i[key], np.ndarray):
                self.shape_check(feed_i, key)
                if self.feed_types_[key] in int_type:
                    if i == 0:
                        int_feed_names.append(key)
                        shape_lst = []
                        if batch == False:
                            feed_i[key] = np.expand_dims(feed_i[key], 0).repeat(
                                1, axis=0)
                        if isinstance(feed_i[key], np.ndarray):
                            shape_lst.extend(list(feed_i[key].shape))
                            int_shape.append(shape_lst)
                        else:
                            int_shape.append(self.feed_shapes_[key])
                        if "{}.lod".format(key) in feed_i:
                            int_lod_slot_batch.append(feed_i["{}.lod".format(
                                key)])
                        else:
                            int_lod_slot_batch.append([])

                    if isinstance(feed_i[key], np.ndarray):
                        int_slot.append(np.ascontiguousarray(feed_i[key]))
                        self.has_numpy_input = True
                    else:
                        int_slot.append(np.ascontiguousarray(feed_i[key]))
                        self.all_numpy_input = False

                elif self.feed_types_[key] in float_type:
                    if i == 0:
                        float_feed_names.append(key)
                        shape_lst = []
                        if batch == False:
                            feed_i[key] = np.expand_dims(feed_i[key], 0).repeat(
                                1, axis=0)
                        if isinstance(feed_i[key], np.ndarray):
                            shape_lst.extend(list(feed_i[key].shape))
                            float_shape.append(shape_lst)
                        else:
                            float_shape.append(self.feed_shapes_[key])
                        if "{}.lod".format(key) in feed_i:
                            float_lod_slot_batch.append(feed_i["{}.lod".format(
                                key)])
                        else:
                            float_lod_slot_batch.append([])

                    if isinstance(feed_i[key], np.ndarray):
                        float_slot.append(np.ascontiguousarray(feed_i[key]))
                        self.has_numpy_input = True
                    else:
                        float_slot.append(np.ascontiguousarray(feed_i[key]))
                        self.all_numpy_input = False
                #if input is string, feed is not numpy.
                elif self.feed_types_[key] in string_type:
                    if i == 0:
                        string_feed_names.append(key)
                        string_shape.append(self.feed_shapes_[key])
                        if "{}.lod".format(key) in feed_i:
                            string_lod_slot_batch.append(feed_i["{}.lod".format(
                                key)])
                        else:
                            string_lod_slot_batch.append([])
                    string_slot.append(feed_i[key])
                    self.has_numpy_input = True
            int_slot_batch.append(int_slot)
            int_lod_slot_batch.append(int_lod_slot)
            float_slot_batch.append(float_slot)
            float_lod_slot_batch.append(float_lod_slot)
            string_slot_batch.append(string_slot)
            string_lod_slot_batch.append(string_lod_slot)

        self.profile_.record('py_prepro_1')
        self.profile_.record('py_client_infer_0')

        result_batch_handle = self.predictorres_constructor()
        if self.all_numpy_input:
            res = self.client_handle_.numpy_predict(
                float_slot_batch, float_feed_names, float_shape,
                float_lod_slot_batch, int_slot_batch, int_feed_names, int_shape,
                int_lod_slot_batch, string_slot_batch, string_feed_names,
                string_shape, string_lod_slot_batch, fetch_names,
                result_batch_handle, self.pid, log_id)
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


class MultiLangClient(object):
    def __init__(self):
        self.channel_ = None
        self.stub_ = None
        self.rpc_timeout_s_ = 2
        self.profile_ = _Profiler()

    def add_variant(self, tag, cluster, variant_weight):
        # TODO
        raise Exception("cannot support ABtest yet")

    def set_rpc_timeout_ms(self, rpc_timeout):
        if self.stub_ is None:
            raise Exception("set timeout must be set after connect.")
        if not isinstance(rpc_timeout, int):
            # for bclient
            raise ValueError("rpc_timeout must be int type.")
        self.rpc_timeout_s_ = rpc_timeout / 1000.0
        timeout_req = multi_lang_general_model_service_pb2.SetTimeoutRequest()
        timeout_req.timeout_ms = rpc_timeout
        resp = self.stub_.SetTimeout(timeout_req)
        return resp.err_code == 0

    def connect(self, endpoints):
        # https://github.com/tensorflow/serving/issues/1382
        options = [('grpc.max_receive_message_length', 512 * 1024 * 1024),
                   ('grpc.max_send_message_length', 512 * 1024 * 1024),
                   ('grpc.lb_policy_name', 'round_robin')]
        # TODO: weight round robin
        g_endpoint = 'ipv4:{}'.format(','.join(endpoints))
        self.channel_ = grpc.insecure_channel(g_endpoint, options=options)
        self.stub_ = multi_lang_general_model_service_pb2_grpc.MultiLangGeneralModelServiceStub(
            self.channel_)
        # get client model config
        get_client_config_req = multi_lang_general_model_service_pb2.GetClientConfigRequest(
        )
        resp = self.stub_.GetClientConfig(get_client_config_req)
        model_config_str = resp.client_config_str
        self._parse_model_config(model_config_str)

    def _flatten_list(self, nested_list):
        for item in nested_list:
            if isinstance(item, (list, tuple)):
                for sub_item in self._flatten_list(item):
                    yield sub_item
            else:
                yield item

    def _parse_model_config(self, model_config_str):
        model_conf = m_config.GeneralModelConfig()
        model_conf = google.protobuf.text_format.Merge(model_config_str,
                                                       model_conf)
        self.feed_names_ = [var.alias_name for var in model_conf.feed_var]
        self.feed_types_ = {}
        self.feed_shapes_ = {}
        self.lod_tensor_set_ = set()
        for i, var in enumerate(model_conf.feed_var):
            self.feed_types_[var.alias_name] = var.feed_type
            self.feed_shapes_[var.alias_name] = var.shape
            if var.is_lod_tensor:
                self.lod_tensor_set_.add(var.alias_name)
        self.fetch_names_ = [var.alias_name for var in model_conf.fetch_var]
        self.fetch_types_ = {}
        for i, var in enumerate(model_conf.fetch_var):
            self.fetch_types_[var.alias_name] = var.fetch_type
            if var.is_lod_tensor:
                self.lod_tensor_set_.add(var.alias_name)

    def _pack_inference_request(self, feed, fetch, is_python, log_id):
        req = multi_lang_general_model_service_pb2.InferenceRequest()
        req.fetch_var_names.extend(fetch)
        req.is_python = is_python
        req.log_id = log_id
        feed_var_names = []
        for key in feed.keys():
            if '.lod' not in key:
                feed_var_names.append(key)
        req.feed_var_names.extend(feed_var_names)
        inst = multi_lang_general_model_service_pb2.FeedInst()
        for name in req.feed_var_names:
            tensor = multi_lang_general_model_service_pb2.Tensor()
            var = feed[name]
            v_type = self.feed_types_[name]
            if is_python:
                data = None
                if isinstance(var, list):
                    if v_type == 0:  # int64
                        data = np.array(var, dtype="int64")
                    elif v_type == 1:  # float32
                        data = np.array(var, dtype="float32")
                    elif v_type == 2:  # int32
                        data = np.array(var, dtype="int32")
                    else:
                        raise Exception("error tensor value type.")
                elif isinstance(var, np.ndarray):
                    data = var
                    if v_type == 0:
                        if data.dtype != 'int64':
                            data = data.astype("int64")
                    elif v_type == 1:
                        if data.dtype != 'float32':
                            data = data.astype("float32")
                    elif v_type == 2:
                        if data.dtype != 'int32':
                            data = data.astype("int32")
                    else:
                        raise Exception("error tensor value type.")
                else:
                    raise Exception("var must be list or ndarray.")
                data = np.ascontiguousarray(data)
                tensor.data = data.tobytes()
            tensor.shape.extend(list(var.shape))
            if "{}.lod".format(name) in feed.keys():
                tensor.lod.extend(feed["{}.lod".format(name)])
            inst.tensor_array.append(tensor)
        req.insts.append(inst)
        return req

    def _unpack_inference_response(self, resp, fetch, is_python,
                                   need_variant_tag):
        if resp.err_code != 0:
            return None
        tag = resp.tag
        multi_result_map = {}
        for model_result in resp.outputs:
            inst = model_result.insts[0]
            result_map = {}
            for i, name in enumerate(fetch):
                var = inst.tensor_array[i]
                v_type = self.fetch_types_[name]
                if is_python:
                    if v_type == 0:  # int64
                        result_map[name] = np.frombuffer(
                            var.data, dtype="int64")
                    elif v_type == 1:  # float32
                        result_map[name] = np.frombuffer(
                            var.data, dtype="float32")
                    else:
                        raise Exception("error type.")
                else:
                    if v_type == 0:  # int64
                        result_map[name] = np.array(
                            list(var.int64_data), dtype="int64")
                    elif v_type == 1:  # float32
                        result_map[name] = np.array(
                            list(var.float_data), dtype="float32")
                    else:
                        raise Exception("error type.")
                result_map[name].shape = list(var.shape)
                if name in self.lod_tensor_set_:
                    result_map["{}.lod".format(name)] = np.array(list(var.lod))
            multi_result_map[model_result.engine_name] = result_map
        ret = None
        if len(resp.outputs) == 1:
            ret = list(multi_result_map.values())[0]
        else:
            ret = multi_result_map

        ret["serving_status_code"] = 0
        return ret if not need_variant_tag else [ret, tag]

    def _done_callback_func(self, fetch, is_python, need_variant_tag):
        def unpack_resp(resp):
            return self._unpack_inference_response(resp, fetch, is_python,
                                                   need_variant_tag)

        return unpack_resp

    def get_feed_names(self):
        return self.feed_names_

    def predict(self,
                feed,
                fetch,
                batch=True,
                need_variant_tag=False,
                asyn=False,
                is_python=True,
                log_id=0):
        if isinstance(feed, dict) is False:
            raise ValueError("Type Error. grpc feed must be dict.")
        if batch is False:
            for key in feed:
                if ".lod" not in key:
                    feed[key] = np.expand_dims(feed[key], 0).repeat(1, axis=0)
        if not asyn:
            try:
                self.profile_.record('py_prepro_0')
                req = self._pack_inference_request(
                    feed, fetch, is_python=is_python, log_id=log_id)
                self.profile_.record('py_prepro_1')

                self.profile_.record('py_client_infer_0')
                resp = self.stub_.Inference(req, timeout=self.rpc_timeout_s_)
                self.profile_.record('py_client_infer_1')

                self.profile_.record('py_postpro_0')
                ret = self._unpack_inference_response(
                    resp,
                    fetch,
                    is_python=is_python,
                    need_variant_tag=need_variant_tag)
                self.profile_.record('py_postpro_1')
                self.profile_.print_profile()
                return ret
            except grpc.RpcError as e:
                return {"serving_status_code": e.code()}
        else:
            req = self._pack_inference_request(
                feed, fetch, is_python=is_python, log_id=log_id)
            call_future = self.stub_.Inference.future(
                req, timeout=self.rpc_timeout_s_)
            return MultiLangPredictFuture(
                call_future,
                self._done_callback_func(
                    fetch,
                    is_python=is_python,
                    need_variant_tag=need_variant_tag))


class MultiLangPredictFuture(object):
    def __init__(self, call_future, callback_func):
        self.call_future_ = call_future
        self.callback_func_ = callback_func

    def result(self):
        try:
            resp = self.call_future_.result()
        except grpc.RpcError as e:
            return {"serving_status_code": e.code()}
        return self.callback_func_(resp)

    def add_done_callback(self, fn):
        def __fn__(call_future):
            assert call_future == self.call_future_
            fn(self)

        self.call_future_.add_done_callback(__fn__)
