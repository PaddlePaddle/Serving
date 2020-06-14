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
import time
import sys

import grpc
from .proto import multi_lang_general_model_service_pb2
sys.path.append(
    os.path.join(os.path.abspath(os.path.dirname(__file__)), 'proto'))
from .proto import grpc_pb2 as grpc_pb2

int_type = 0
float_type = 1


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

    def load_client_config(self, path):
        from .serving_client import PredictorClient
        model_conf = m_config.GeneralModelConfig()
        f = open(path, 'r')
        model_conf = google.protobuf.text_format.Merge(
            str(f.read()), model_conf)

        # load configuraion here
        # get feed vars, fetch vars
        # get feed shapes, feed types
        # map feed names to index
        self.client_handle_ = PredictorClient()
        self.client_handle_.init(path)
        if "FLAGS_max_body_size" not in os.environ:
            os.environ["FLAGS_max_body_size"] = str(512 * 1024 * 1024)
        read_env_flags = ["profile_client", "profile_server", "max_body_size"]
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

    def connect(self, endpoints=None):
        # check whether current endpoint is available
        # init from client config
        # create predictor here
        if endpoints is None:
            if self.predictor_sdk_ is None:
                raise ValueError(
                    "You must set the endpoints parameter or use add_variant function to create a variant."
                )
        else:
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

    def predict(self, feed=None, fetch=None, need_variant_tag=False):
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

        for i, feed_i in enumerate(feed_batch):
            int_slot = []
            float_slot = []
            for key in feed_i:
                if key not in self.feed_names_:
                    raise ValueError("Wrong feed name: {}.".format(key))
                #if not isinstance(feed_i[key], np.ndarray):
                self.shape_check(feed_i, key)
                if self.feed_types_[key] == int_type:
                    if i == 0:
                        int_feed_names.append(key)
                        if isinstance(feed_i[key], np.ndarray):
                            int_shape.append(list(feed_i[key].shape))
                        else:
                            int_shape.append(self.feed_shapes_[key])
                    if isinstance(feed_i[key], np.ndarray):
                        int_slot.append(feed_i[key])
                        self.has_numpy_input = True
                    else:
                        int_slot.append(feed_i[key])
                        self.all_numpy_input = False
                elif self.feed_types_[key] == float_type:
                    if i == 0:
                        float_feed_names.append(key)
                        if isinstance(feed_i[key], np.ndarray):
                            float_shape.append(list(feed_i[key].shape))
                        else:
                            float_shape.append(self.feed_shapes_[key])
                    if isinstance(feed_i[key], np.ndarray):
                        float_slot.append(feed_i[key])
                        self.has_numpy_input = True
                    else:
                        float_slot.append(feed_i[key])
                        self.all_numpy_input = False
            int_slot_batch.append(int_slot)
            float_slot_batch.append(float_slot)

        self.profile_.record('py_prepro_1')
        self.profile_.record('py_client_infer_0')

        result_batch_handle = self.predictorres_constructor()
        if self.all_numpy_input:
            res = self.client_handle_.numpy_predict(
                float_slot_batch, float_feed_names, float_shape, int_slot_batch,
                int_feed_names, int_shape, fetch_names, result_batch_handle,
                self.pid)
        elif self.has_numpy_input == False:
            res = self.client_handle_.batch_predict(
                float_slot_batch, float_feed_names, float_shape, int_slot_batch,
                int_feed_names, int_shape, fetch_names, result_batch_handle,
                self.pid)
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
                if self.fetch_names_to_type_[name] == int_type:
                    # result_map[name] will be py::array(numpy array)
                    result_map[name] = result_batch_handle.get_int64_by_name(
                        mi, name)
                    shape = result_batch_handle.get_shape(mi, name)
                    result_map[name].shape = shape
                    if name in self.lod_tensor_set:
                        result_map["{}.lod".format(
                            name)] = result_batch_handle.get_lod(mi, name)
                elif self.fetch_names_to_type_[name] == float_type:
                    result_map[name] = result_batch_handle.get_float_by_name(
                        mi, name)
                    shape = result_batch_handle.get_shape(mi, name)
                    result_map[name].shape = shape
                    if name in self.lod_tensor_set:
                        result_map["{}.lod".format(
                            name)] = result_batch_handle.get_lod(mi, name)
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
        self._config = None

    def load_client_config(self, path):
        if not isinstance(path, str):
            raise Exception("GClient only supports multi-model temporarily")
        with open(path, 'r') as f:
            proto_txt = str(f.read())
        
        self._parse_model_config(proto_txt)

    def _load_client_config(self, stub):
        req= grpc_pb2.ServingConfig()
        self._config  = self.stub_.get_client_proto_text(req)
        self._parse_model_config(config.proto_txt)

    def connect(self, endpoint, use_remote_config=True):
        self.channel_ = grpc.insecure_channel(endpoint[0])  #TODO
        self.stub_ = grpc_pb2.MultiLangGeneralModelServiceStub(
            self.channel_)

        if use_remote_config:
            self._load_client_config(stub)

    def _flatten_list(self, nested_list):
        for item in nested_list:
            if isinstance(item, (list, tuple)):
                for sub_item in self._flatten_list(item):
                    yield sub_item
            else:
                yield item

    def _parse_model_config(self, proto_txt):
        model_conf = m_config.GeneralModelConfig()
        model_conf = google.protobuf.text_format.Merge(proto_txt, model_conf)
        self.feed_names_ = [var.alias_name for var in model_conf.feed_var]
        self.feed_types_ = {}
        self.feed_shapes_ = {}
        self.fetch_names_ = [var.alias_name for var in model_conf.fetch_var]
        self.fetch_types_ = {}
        self.lod_tensor_set_ = set()
        for i, var in enumerate(model_conf.feed_var):
            self.feed_types_[var.alias_name] = var.feed_type
            self.feed_shapes_[var.alias_name] = var.shape
            if var.is_lod_tensor:
                self.lod_tensor_set_.add(var.alias_name)
            else:
                counter = 1
                for dim in self.feed_shapes_[var.alias_name]:
                    counter *= dim
        for i, var in enumerate(model_conf.fetch_var):
            self.fetch_types_[var.alias_name] = var.fetch_type
            if var.is_lod_tensor:
                self.lod_tensor_set_.add(var.alias_name)

    def _pack_feed_data(self, feed, fetch, is_python):
        req = multi_lang_general_model_service_pb2.Request()
        req.fetch_var_names.extend(fetch)
        req.feed_var_names.extend(feed.keys())
        req.is_python = is_python
        feed_batch = None
        if isinstance(feed, dict):
            feed_batch = [feed]
        elif isinstance(feed, list):
            feed_batch = feed
        else:
            raise Exception("{} not support".format(type(feed)))
        init_feed_names = False
        for feed_data in feed_batch:
            inst = multi_lang_general_model_service_pb2.FeedInst()
            for name in req.feed_var_names:
                tensor = multi_lang_general_model_service_pb2.Tensor()
                var = feed_data[name]
                v_type = self.feed_types_[name]
                if is_python:
                    data = None
                    if isinstance(var, list):
                        if v_type == 0:  # int64
                            data = np.array(var, dtype="int64")
                        elif v_type == 1:  # float32
                            data = np.array(var, dtype="float32")
                        else:
                            raise Exception("error type.")
                    else:
                        data = var
                        if var.dtype == "float64":
                            data = data.astype("float32")
                    tensor.data = data.tobytes()
                else:
                    if v_type == 0:  # int64
                        if isinstance(var, np.ndarray):
                            tensor.int64_data.extend(var.reshape(-1).tolist())
                        else:
                            tensor.int64_data.extend(self._flatten_list(var))
                    elif v_type == 1:  # float32
                        if isinstance(var, np.ndarray):
                            tensor.float_data.extend(var.reshape(-1).tolist())
                        else:
                            tensor.float_data.extend(self._flatten_list(var))
                    else:
                        raise Exception("error type.")
                if isinstance(var, np.ndarray):
                    tensor.shape.extend(list(var.shape))
                else:
                    tensor.shape.extend(self.feed_shapes_[name])
                inst.tensor_array.append(tensor)
            req.insts.append(inst)
        return req

    def _unpack_resp(self, resp, fetch, is_python, need_variant_tag):
        result_map = {}
        inst = resp.outputs[0].insts[0]
        tag = resp.tag
        for i, name in enumerate(fetch):
            var = inst.tensor_array[i]
            v_type = self.fetch_types_[name]
            if is_python:
                if v_type == 0:  # int64
                    result_map[name] = np.frombuffer(var.data, dtype="int64")
                elif v_type == 1:  # float32
                    result_map[name] = np.frombuffer(var.data, dtype="float32")
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
        return result_map if not need_variant_tag else [result_map, tag]

    def _done_callback_func(self, fetch, is_python, need_variant_tag):
        def unpack_resp(resp):
            return self._unpack_resp(resp, fetch, is_python, need_variant_tag)

        return unpack_resp

    def predict(self,
                feed,
                fetch,
                need_variant_tag=False,
                asyn=False,
                is_python=True):
        req = self._pack_feed_data(feed, fetch, is_python=is_python)
        if not asyn:
            resp = self.stub_.inference(req)
            return self._unpack_resp(
                resp,
                fetch,
                is_python=is_python,
                need_variant_tag=need_variant_tag)
        else:
            call_future = self.stub_.inference.future(req)
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
        resp = self.call_future_.result()
        return self.callback_func_(resp)
