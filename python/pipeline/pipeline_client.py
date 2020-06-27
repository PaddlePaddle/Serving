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
# pylint: disable=doc-string-missing
import grpc
import numpy as np
from .proto import pipeline_service_pb2
from .proto import pipeline_service_pb2_grpc


class PipelineClient(object):
    def __init__(self):
        self._channel = None

    def connect(self, endpoint):
        self._channel = grpc.insecure_channel(endpoint)
        self._stub = pipeline_service_pb2_grpc.PipelineServiceStub(
            self._channel)

    def _pack_data_for_infer(self, feed_dict):
        req = pipeline_service_pb2.Request()
        for key, value in feed_dict.items():
            if not isinstance(value, str):
                raise TypeError("only str type is supported.")
            req.key.append(key)
            req.value.append(value)
        return req

    def predict(self, feed_dict, fetch):
        if not isinstance(feed_dict, dict):
            raise TypeError(
                "feed must be dict type with format: {name: value}.")
        if not isinstance(fetch, list):
            raise TypeError(
                "fetch_with_type must be list type with format: [name].")
        req = self._pack_data_for_infer(feed_dict)
        resp = self._stub.inference(req)
        if resp.ecode != 0:
            return {"ecode": resp.ecode, "error_info": resp.error_info}
        fetch_map = {"ecode": resp.ecode}
        for idx, key in enumerate(resp.key):
            if key not in fetch:
                continue
            fetch_map[key] = resp.value[idx]
        return fetch_map
