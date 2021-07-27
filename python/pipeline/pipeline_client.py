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

    def _pack_request_package(self, feed_dict, profile):
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
        for key, value in feed_dict.items():
            req.key.append(key)

            if (sys.version_info.major == 2 and isinstance(value,
                                                           (str, unicode)) or
                ((sys.version_info.major == 3) and isinstance(value, str))):
                req.value.append(value)
                continue

            if isinstance(value, np.ndarray):
                req.value.append(value.__repr__())
            elif isinstance(value, list):
                req.value.append(np.array(value).__repr__())
            else:
                raise TypeError("only str and np.ndarray type is supported: {}".
                                format(type(value)))
        if profile:
            req.key.append(self._profile_key)
            req.value.append(self._profile_value)
        return req

    def _unpack_response_package(self, resp, fetch):
        return resp

    def predict(self, feed_dict, fetch=None, asyn=False, profile=False):
        if not isinstance(feed_dict, dict):
            raise TypeError(
                "feed must be dict type with format: {name: value}.")
        if fetch is not None and not isinstance(fetch, list):
            raise TypeError("fetch must be list type with format: [name].")
        req = self._pack_request_package(feed_dict, profile)
        if not asyn:
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
