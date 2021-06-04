# Copyright (c) 2021 PaddlePaddle Authors. All Rights Reserved.
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

import sys
import os
import numpy as np
import google.protobuf.text_format

from .proto import general_model_config_pb2 as m_config
from .proto import multi_lang_general_model_service_pb2
sys.path.append(
    os.path.join(os.path.abspath(os.path.dirname(__file__)), 'proto'))
from .proto import multi_lang_general_model_service_pb2_grpc


class MultiLangServerServiceServicer(multi_lang_general_model_service_pb2_grpc.
                                     MultiLangGeneralModelServiceServicer):
    def __init__(self, model_config_path_list, is_multi_model, endpoints):
        self.is_multi_model_ = is_multi_model
        self.model_config_path_list = model_config_path_list
        self.endpoints_ = endpoints
        self._init_bclient(self.model_config_path_list, self.endpoints_)
        self._parse_model_config(self.model_config_path_list)

    def _init_bclient(self, model_config_path_list, endpoints, timeout_ms=None):
        file_path_list = []
        for single_model_config in model_config_path_list:
            if os.path.isdir(single_model_config):
                file_path_list.append("{}/serving_server_conf.prototxt".format(
                    single_model_config))
            elif os.path.isfile(single_model_config):
                file_path_list.append(single_model_config)
        from paddle_serving_client import Client
        self.bclient_ = Client()
        if timeout_ms is not None:
            self.bclient_.set_rpc_timeout_ms(timeout_ms)
        self.bclient_.load_client_config(file_path_list)
        self.bclient_.connect(endpoints)

    def _parse_model_config(self, model_config_path_list):
        if isinstance(model_config_path_list, str):
            model_config_path_list = [model_config_path_list]
        elif isinstance(model_config_path_list, list):
            pass

        file_path_list = []
        for single_model_config in model_config_path_list:
            if os.path.isdir(single_model_config):
                file_path_list.append("{}/serving_server_conf.prototxt".format(
                    single_model_config))
            elif os.path.isfile(single_model_config):
                file_path_list.append(single_model_config)
        model_conf = m_config.GeneralModelConfig()
        f = open(file_path_list[0], 'r')
        model_conf = google.protobuf.text_format.Merge(
            str(f.read()), model_conf)
        self.feed_names_ = [var.alias_name for var in model_conf.feed_var]
        self.feed_types_ = {}
        self.feed_shapes_ = {}
        self.lod_tensor_set_ = set()
        for i, var in enumerate(model_conf.feed_var):
            self.feed_types_[var.alias_name] = var.feed_type
            self.feed_shapes_[var.alias_name] = var.shape
            if var.is_lod_tensor:
                self.lod_tensor_set_.add(var.alias_name)
        if len(file_path_list) > 1:
            model_conf = m_config.GeneralModelConfig()
            f = open(file_path_list[-1], 'r')
            model_conf = google.protobuf.text_format.Merge(
                str(f.read()), model_conf)

        self.fetch_names_ = [var.alias_name for var in model_conf.fetch_var]
        self.fetch_types_ = {}
        for i, var in enumerate(model_conf.fetch_var):
            self.fetch_types_[var.alias_name] = var.fetch_type
            if var.is_lod_tensor:
                self.lod_tensor_set_.add(var.alias_name)

    def _flatten_list(self, nested_list):
        for item in nested_list:
            if isinstance(item, (list, tuple)):
                for sub_item in self._flatten_list(item):
                    yield sub_item
            else:
                yield item

    def _unpack_inference_request(self, request):
        feed_names = list(request.feed_var_names)
        fetch_names = list(request.fetch_var_names)
        is_python = request.is_python
        log_id = request.log_id
        feed_batch = []
        for feed_inst in request.insts:
            feed_dict = {}
            for idx, name in enumerate(feed_names):
                var = feed_inst.tensor_array[idx]
                v_type = self.feed_types_[name]
                data = None
                if is_python:
                    if v_type == 0:  # int64
                        data = np.frombuffer(var.data, dtype="int64")
                    elif v_type == 1:  # float32
                        data = np.frombuffer(var.data, dtype="float32")
                    elif v_type == 2:  # int32
                        data = np.frombuffer(var.data, dtype="int32")
                    else:
                        raise Exception("error type.")
                else:
                    if v_type == 0:  # int64
                        data = np.array(list(var.int64_data), dtype="int64")
                    elif v_type == 1:  # float32
                        data = np.array(list(var.float_data), dtype="float32")
                    elif v_type == 2:  # int32
                        data = np.array(list(var.int_data), dtype="int32")
                    else:
                        raise Exception("error type.")
                data.shape = list(feed_inst.tensor_array[idx].shape)
                feed_dict[name] = np.ascontiguousarray(data)
                if len(var.lod) > 0:
                    feed_dict["{}.lod".format(name)] = var.lod
            feed_batch.append(feed_dict)
        return feed_batch, fetch_names, is_python, log_id

    def _pack_inference_response(self, ret, fetch_names, is_python):
        resp = multi_lang_general_model_service_pb2.InferenceResponse()
        if ret is None:
            resp.err_code = 1
            return resp
        results, tag = ret
        resp.tag = tag
        resp.err_code = 0

        if not self.is_multi_model_:
            results = {'general_infer_0': results}
        for model_name, model_result in results.items():
            model_output = multi_lang_general_model_service_pb2.ModelOutput()
            inst = multi_lang_general_model_service_pb2.FetchInst()
            for idx, name in enumerate(fetch_names):
                tensor = multi_lang_general_model_service_pb2.Tensor()
                v_type = self.fetch_types_[name]
                if is_python:
                    tensor.data = model_result[name].tobytes()
                else:
                    if v_type == 0:  # int64
                        tensor.int64_data.extend(model_result[name].reshape(-1)
                                                 .tolist())
                    elif v_type == 1:  # float32
                        tensor.float_data.extend(model_result[name].reshape(-1)
                                                 .tolist())
                    elif v_type == 2:  # int32
                        tensor.int_data.extend(model_result[name].reshape(-1)
                                               .tolist())
                    else:
                        raise Exception("error type.")
                tensor.shape.extend(list(model_result[name].shape))
                if "{}.lod".format(name) in model_result:
                    tensor.lod.extend(model_result["{}.lod".format(name)]
                                      .tolist())
                inst.tensor_array.append(tensor)
            model_output.insts.append(inst)
            model_output.engine_name = model_name
            resp.outputs.append(model_output)
        return resp

    def SetTimeout(self, request, context):
        # This porcess and Inference process cannot be operate at the same time.
        # For performance reasons, do not add thread lock temporarily.
        timeout_ms = request.timeout_ms
        self._init_bclient(self.model_config_path_list, self.endpoints_,
                           timeout_ms)
        resp = multi_lang_general_model_service_pb2.SimpleResponse()
        resp.err_code = 0
        return resp

    def Inference(self, request, context):
        feed_batch, fetch_names, is_python, log_id \
                = self._unpack_inference_request(request)
        ret = self.bclient_.predict(
            feed=feed_batch,
            fetch=fetch_names,
            batch=True,
            need_variant_tag=True,
            log_id=log_id)
        return self._pack_inference_response(ret, fetch_names, is_python)

    def GetClientConfig(self, request, context):
        #model_config_path_list is list right now.
        #dict should be added when graphMaker is used.
        resp = multi_lang_general_model_service_pb2.GetClientConfigResponse()
        model_config_str = []
        for single_model_config in self.model_config_path_list:
            if os.path.isdir(single_model_config):
                with open("{}/serving_server_conf.prototxt".format(
                        single_model_config)) as f:
                    model_config_str.append(str(f.read()))
            elif os.path.isfile(single_model_config):
                with open(single_model_config) as f:
                    model_config_str.append(str(f.read()))
        resp.client_config_str = model_config_str[0]
        return resp
