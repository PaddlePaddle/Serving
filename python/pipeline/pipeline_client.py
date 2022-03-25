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
import sys
import time
import numpy as np
from numpy import *
import logging
import functools
import json
import socket
from .channel import ChannelDataErrcode
from .proto import pipeline_service_pb2
from .proto import pipeline_service_pb2_grpc
import six
from io import BytesIO
_LOGGER = logging.getLogger(__name__)


class PipelineClient(object):
    """
    PipelineClient provides the basic capabilities of the pipeline SDK
    """

    def __init__(self):
        self._channel = None
        self._profile_key = "pipeline.profile"
        self._profile_value = "1"

    def connect(self, endpoints):
        options = [('grpc.max_receive_message_length', 512 * 1024 * 1024),
                   ('grpc.max_send_message_length', 512 * 1024 * 1024),
                   ('grpc.lb_policy_name', 'round_robin')]
        g_endpoint = 'ipv4:{}'.format(','.join(endpoints))
        self._channel = grpc.insecure_channel(g_endpoint, options=options)
        self._stub = pipeline_service_pb2_grpc.PipelineServiceStub(
            self._channel)

    def _pack_request_package(self, feed_dict, pack_tensor_format,
                              use_tensor_bytes, profile):
        req = pipeline_service_pb2.Request()

        logid = feed_dict.get("logid")
        if logid is None:
            req.logid = 0
        else:
            if sys.version_info.major == 2:
                req.logid = long(logid)
            elif sys.version_info.major == 3:
                req.logid = int(logid)
            feed_dict.pop("logid")

        clientip = feed_dict.get("clientip")
        if clientip is None:
            hostname = socket.gethostname()
            ip = socket.gethostbyname(hostname)
            req.clientip = ip
        else:
            req.clientip = clientip
            feed_dict.pop("clientip")

        np.set_printoptions(threshold=sys.maxsize)
        if pack_tensor_format is False:
            # pack string key/val format
            for key, value in feed_dict.items():
                req.key.append(key)

                if (sys.version_info.major == 2 and
                        isinstance(value, (str, unicode)) or
                    ((sys.version_info.major == 3) and isinstance(value, str))):
                    req.value.append(value)
                    continue

                if isinstance(value, np.ndarray):
                    req.value.append(value.__repr__())
                elif isinstance(value, list):
                    req.value.append(np.array(value).__repr__())
                else:
                    raise TypeError(
                        "only str and np.ndarray type is supported: {}".format(
                            type(value)))

            if profile:
                req.key.append(self._profile_key)
                req.value.append(self._profile_value)
        else:
            # pack tensor format
            for key, value in feed_dict.items():

                # skipping the lod feed_var.
                # The declare of lod feed_var must be hebind the feed_var.
                if ".lod" in key:
                    continue

                one_tensor = req.tensors.add()
                one_tensor.name = key

                if isinstance(value, str):
                    one_tensor.str_data.append(value)
                    one_tensor.elem_type = 12  #12 => string in proto
                    continue

                if isinstance(value, np.ndarray):
                    # copy shape
                    _LOGGER.debug(
                        "key:{}, use_tensor_bytes:{}, value.shape:{}, value.dtype:{}".
                        format(key, use_tensor_bytes, value.shape, value.dtype))
                    for one_dim in value.shape:
                        one_tensor.shape.append(one_dim)

                    # set lod info, must be list type.
                    lod_key = key + ".lod"
                    if lod_key in feed_dict:
                        lod_list = feed_dict.get(lod_key)
                        if lod_list is not None:
                            one_tensor.lod.extend(lod_list)

                    # packed into bytes
                    if use_tensor_bytes is True:
                        np_bytes = BytesIO()
                        np.save(np_bytes, value, allow_pickle=True)
                        one_tensor.byte_data = np_bytes.getvalue()
                        one_tensor.elem_type = 13  #13 => bytes in proto
                        continue

                    flat_value = value.flatten().tolist()
                    # copy data
                    if value.dtype == "int64":
                        one_tensor.int64_data.extend(flat_value)
                        one_tensor.elem_type = 0
                    elif value.dtype == "float32":
                        one_tensor.float_data.extend(flat_value)
                        one_tensor.elem_type = 1
                    elif value.dtype == "int32":
                        one_tensor.int_data.extend(flat_value)
                        one_tensor.elem_type = 2
                    elif value.dtype == "float64":
                        one_tensor.float64_data.extend(flat_value)
                        one_tensor.elem_type = 3
                    elif value.dtype == "int16":
                        one_tensor.int_data.extend(flat_value)
                        one_tensor.elem_type = 4
                    elif value.dtype == "float16":
                        one_tensor.float_data.extend(flat_value)
                        one_tensor.elem_type = 5
                    elif value.dtype == "uint16":
                        one_tensor.uint32_data.extend(flat_value)
                        one_tensor.elem_type = 6
                    elif value.dtype == "uint8":
                        one_tensor.uint32_data.extend(flat_value)
                        one_tensor.elem_type = 7
                    elif value.dtype == "int8":
                        one_tensor.int_data.extend(flat_value)
                        one_tensor.elem_type = 8
                    elif value.dtype == "bool":
                        one_tensor.bool_data.extend(flat_value)
                        one_tensor.elem_type = 9
                    else:
                        _LOGGER.error(
                            "value type {} of tensor {} is not supported.".
                            format(value.dtype, key))
                else:
                    raise TypeError(
                        "only str and np.ndarray type is supported: {}".format(
                            type(value)))
        return req

    def _unpack_response_package(self, resp, fetch):
        return resp

    def predict(self,
                feed_dict,
                fetch=None,
                asyn=False,
                pack_tensor_format=False,
                use_tensor_bytes=False,
                profile=False,
                log_id=0):
        if not isinstance(feed_dict, dict):
            raise TypeError(
                "feed must be dict type with format: {name: value}.")
        if fetch is not None and not isinstance(fetch, list):
            raise TypeError("fetch must be list type with format: [name].")
        print("PipelineClient::predict pack_data time:{}".format(time.time()))
        req = self._pack_request_package(feed_dict, pack_tensor_format,
                                         use_tensor_bytes, profile)
        req.logid = log_id
        if not asyn:
            print("PipelineClient::predict before time:{}".format(time.time()))
            resp = self._stub.inference(req)
            return self._unpack_response_package(resp, fetch)
        else:
            call_future = self._stub.inference.future(req)
            return PipelinePredictFuture(
                call_future,
                functools.partial(
                    self._unpack_response_package, fetch=fetch))


class PipelinePredictFuture(object):
    def __init__(self, call_future, callback_func):
        self.call_future_ = call_future
        self.callback_func_ = callback_func

    def result(self):
        resp = self.call_future_.result()
        return self.callback_func_(resp)
