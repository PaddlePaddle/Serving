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

import requests
import json
import numpy as np
import os
from .proto import general_model_config_pb2 as m_config
import google.protobuf.text_format
import gzip
from collections import Iterable
import base64
import sys
import re

import grpc
from .proto import general_model_service_pb2
sys.path.append(
    os.path.join(os.path.abspath(os.path.dirname(__file__)), 'proto'))
from .proto import general_model_service_pb2_grpc
#param 'type'(which is in feed_var or fetch_var) = 0 means dataType is int64
#param 'type'(which is in feed_var or fetch_var) = 1 means dataType is float32
#param 'type'(which is in feed_var or fetch_var) = 2 means dataType is int32
#param 'type'(which is in feed_var or fetch_var) = 20 means dataType is string(also called bytes in proto)
int64_type = 0
float32_type = 1
int32_type = 2
bytes_type = 20
# this is corresponding to the proto
proto_data_key_list = {
    0: "int64_data",
    1: "float_data",
    2: "int_data",
    20: "data"
}


def list_flatten(items, ignore_types=(str, bytes)):
    for x in items:
        if isinstance(x, Iterable) and not isinstance(x, ignore_types):
            yield from list_flatten(x)
        else:
            yield x


def data_bytes_number(datalist):
    total_bytes_number = 0
    if isinstance(datalist, list):
        if len(datalist) == 0:
            return total_bytes_number
        else:
            for data in datalist:
                if isinstance(data, str):
                    total_bytes_number = total_bytes_number + len(data)
                else:
                    total_bytes_number = total_bytes_number + 4 * len(datalist)
                    break
    else:
        raise ValueError(
            "In the Function data_bytes_number(), data must be list.")
    return total_bytes_number


# 此文件名，暂时为httpclient.py，待后续测试后考虑是否替换client.py
# 默认使用http方式，默认使用Proto in HTTP-body
# 如果想使用JSON in HTTP-body, set_http_proto(False)
# Predict()是包装类http_client_predict/grpc_client_predict
# 可以直接调用需要的http_client_predict/grpc_client_predict
# 例如，如果想使用GRPC方式，set_use_grpc_client(True)
# 或者直接调用grpc_client_predict()
class HttpClient(object):
    def __init__(self,
                 ip="127.0.0.1",
                 port="9393",
                 service_name="/GeneralModelService/inference"):
        self.feed_names_ = []
        self.feed_real_names = []
        self.fetch_names_ = []
        self.feed_shapes_ = {}
        self.feed_types_ = {}
        self.feed_names_to_idx_ = {}
        self.timeout_ms = 20000
        self.ip = ip
        self.port = port
        self.server_port = port
        self.service_name = service_name
        self.key = None
        self.try_request_gzip = False
        self.try_response_gzip = False
        self.total_data_number = 0
        self.headers = {}
        self.http_proto = True
        self.headers["Content-Type"] = "application/proto"
        self.max_body_size = 512 * 1024 * 1024
        self.use_grpc_client = False
        self.http_s = "http://"

        # 使用连接池能够不用反复建立连接
        self.requests_session = requests.session()
        # 初始化grpc_stub
        options = [('grpc.max_receive_message_length', self.max_body_size),
                   ('grpc.max_send_message_length', self.max_body_size)]

        endpoints = [self.ip + ":" + self.server_port]
        g_endpoint = 'ipv4:{}'.format(','.join(endpoints))
        self.channel_ = grpc.insecure_channel(g_endpoint, options=options)
        self.stub_ = general_model_service_pb2_grpc.GeneralModelServiceStub(
            self.channel_)

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
        model_conf = m_config.GeneralModelConfig()
        f = open(file_path_list[0], 'r')
        model_conf = google.protobuf.text_format.Merge(
            str(f.read()), model_conf)

        # load configuraion here
        # get feed vars, fetch vars
        # get feed shapes, feed types
        # map feed names to index
        self.feed_names_ = [var.alias_name for var in model_conf.feed_var]
        self.feed_real_names = [var.name for var in model_conf.feed_var]
        self.feed_names_to_idx_ = {}  #this is useful
        self.lod_tensor_set = set()
        self.feed_tensor_len = {}  #this is only used for shape check
        self.key = None

        for i, var in enumerate(model_conf.feed_var):
            self.feed_names_to_idx_[var.alias_name] = i
            self.feed_types_[var.alias_name] = var.feed_type
            self.feed_shapes_[var.alias_name] = [dim for dim in var.shape]

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

    def set_max_body_size(self, max_body_size):
        self.max_body_size = max_body_size

    def set_timeout_ms(self, timeout_ms):
        if not isinstance(timeout_ms, int):
            raise ValueError("timeout_ms must be int type.")
        else:
            self.timeout_ms = timeout_ms

    def set_max_retries(self, retry_times=3):
        if not isinstance(retry_times, int):
            raise ValueError("retry_times must be int type.")
        else:
            self.requests_session.mount(
                self.http_s, HTTPAdapter(max_retries=retry_times))

    def set_service_name(self, service_name):
        self.service_name = service_name

    def connect(self, url=None, encryption=False):
        if isinstance(url, (list, tuple)):
            if len(url) > 1:
                raise ValueError("HttpClient only support 1 endpoint")
            else:
                url = url[0]
        if isinstance(url, str):
            if url.startswith("https://"):
                url = url[8:]
                self.http_s = "https://"
            if url.startswith("http://"):
                url = url[7:]
                self.http_s = "http://"
            url_parts = url.split(':')
            if len(url_parts) != 2 or self.check_ip(url_parts[0]) == False:
                raise ValueError(
                    "url not right, it should be like 127.0.0.1:9393 or http://127.0.0.1:9393"
                )
            else:
                self.ip = url_parts[0]
                self.port = url_parts[1]
                self.server_port = url_parts[1]
        if encryption:
            self.get_serving_port()
        if self.use_grpc_client:
            self.init_grpc_stub()

    def check_ip(self, ipAddr):
        compile_ip = re.compile(
            '^(1\d{2}|2[0-4]\d|25[0-5]|[1-9]\d|[1-9])\.(1\d{2}|2[0-4]\d|25[0-5]|[1-9]\d|\d)\.(1\d{2}|2[0-4]\d|25[0-5]|[1-9]\d|\d)\.(1\d{2}|2[0-4]\d|25[0-5]|[1-9]\d|\d)$'
        )
        if compile_ip.match(ipAddr):
            return True
        else:
            return False

    def add_http_headers(self, headers):
        if isinstance(headers, dict):
            self.headers.update(headers)
        else:
            print("headers must be a dict")

    def set_request_compress(self, try_request_gzip):
        self.try_request_gzip = try_request_gzip

    def set_response_compress(self, try_response_gzip):
        self.try_response_gzip = try_response_gzip

    def set_http_proto(self, http_proto):
        self.http_proto = http_proto
        if self.http_proto:
            self.headers["Content-Type"] = "application/proto"
        else:
            self.headers["Content-Type"] = "application/json"

    def set_use_grpc_client(self, use_grpc_client):
        self.use_grpc_client = use_grpc_client

    # use_key is the function of encryption.
    def use_key(self, key_filename):
        with open(key_filename, "rb") as f:
            self.key = f.read()

    def get_serving_port(self):
        encrypt_url = self.http_s + str(self.ip) + ":" + str(self.port)
        if self.key is not None:
            req = json.dumps({"key": base64.b64encode(self.key).decode()})
        else:
            req = json.dumps({})
        with requests.post(
                encrypt_url, data=req, timeout=self.timeout_ms / 1000) as r:
            result = r.json()
            if "endpoint_list" not in result:
                raise ValueError("server not ready")
            else:
                self.server_port = str(result["endpoint_list"][0])
                print("rpc port is ", self.server_port)

    def get_feed_names(self):
        return self.feed_names_

    def get_fetch_names(self):
        return self.fetch_names_

    def get_legal_fetch(self, fetch):

        fetch_list = []
        if isinstance(fetch, str):
            fetch_list = [fetch]
        elif isinstance(fetch, (list, tuple)):
            fetch_list = fetch
        elif fetch == None:
            pass
        else:
            raise ValueError("Fetch only accepts string/list/tuple of string")

        fetch_names = []
        for key in fetch_list:
            if key in self.fetch_names_:
                fetch_names.append(key)
        return fetch_names

    def get_feedvar_dict(self, feed):
        if feed is None:
            raise ValueError("You should specify feed for prediction")
        feed_dict = {}
        if isinstance(feed, dict):
            feed_dict = feed
        elif isinstance(feed, (list, str, tuple)):
            # feed = [dict]
            if len(feed) == 1 and isinstance(feed[0], dict):
                feed_dict = feed[0]
                return feed_dict
            # if input is a list or str or tuple, and the number of feed_var is 1.
            # create a feed_dict { key = feed_var_name, value = list}
            if len(self.feed_names_) == 1:
                feed_dict[self.feed_names_[0]] = feed
            elif len(self.feed_names_) > 1:
                if isinstance(feed, str):
                    raise ValueError(
                        "input is a str, but we got 2+ feed_var, don`t know how to divide the string"
                    )
                # feed is a list or tuple
                elif len(self.feed_names_) == len(feed):
                    for index in range(len(feed)):
                        feed_dict[self.feed_names_[index]] = feed[index]
                else:
                    raise ValueError("len(feed) ≠ len(feed_var), error")
            else:
                raise ValueError("we got feed, but feed_var is None")

        else:
            raise ValueError("Feed only accepts dict/str/list/tuple")

        return feed_dict

    def process_json_data(self, feed_dict, fetch_list, batch, log_id):
        Request = {}
        Request["fetch_var_names"] = fetch_list
        Request["log_id"] = int(log_id)
        Request["tensor"] = []
        for key in feed_dict:
            if ".lod" not in key and key not in self.feed_names_:
                raise ValueError("Wrong feed name: {}.".format(key))
            if ".lod" in key:
                continue

            tensor_dict = self.process_tensor(key, feed_dict, batch)
            data_key = tensor_dict["data_key"]
            data_value = tensor_dict["data_value"]

            tensor = {}
            tensor[data_key] = data_value
            tensor["shape"] = tensor_dict["shape"]
            tensor["elem_type"] = tensor_dict["elem_type"]
            tensor["name"] = tensor_dict["name"]
            tensor["alias_name"] = tensor_dict["alias_name"]
            if "lod" in tensor_dict:
                tensor["lod"] = tensor_dict["lod"]
            Request["tensor"].append(tensor)

        # request
        postData = json.dumps(Request)
        return postData

    def process_proto_data(self, feed_dict, fetch_list, batch, log_id):
        req = general_model_service_pb2.Request()
        req.fetch_var_names.extend(fetch_list)
        req.log_id = log_id

        for key in feed_dict:
            tensor = general_model_service_pb2.Tensor()
            if ".lod" not in key and key not in self.feed_names_:
                raise ValueError("Wrong feed name: {}.".format(key))
            if ".lod" in key:
                continue

            tensor_dict = self.process_tensor(key, feed_dict, batch)

            tensor.shape.extend(tensor_dict["shape"])
            tensor.name = tensor_dict["name"]
            tensor.alias_name = tensor_dict["alias_name"]
            tensor.elem_type = tensor_dict["elem_type"]
            if "lod" in tensor_dict:
                tensor.lod.extend(tensor_dict["lod"])

            if tensor_dict["data_key"] == "int64_data":
                tensor.int64_data.extend(tensor_dict["data_value"])
            elif tensor_dict["data_key"] == "float_data":
                tensor.float_data.extend(tensor_dict["data_value"])
            elif tensor_dict["data_key"] == "int_data":
                tensor.int_data.extend(tensor_dict["data_value"])
            elif tensor_dict["data_key"] == "data":
                tensor.data.extend(tensor_dict["data_value"])
            else:
                raise ValueError(
                    "tensor element_type must be one of [int64_data,float_data,int_data,data]."
                )

            req.tensor.append(tensor)

        return req

    def process_tensor(self, key, feed_dict, batch):
        lod = []
        if "{}.lod".format(key) in feed_dict:
            lod = feed_dict["{}.lod".format(key)]
        shape = self.feed_shapes_[key].copy()
        elem_type = self.feed_types_[key]
        data_value = feed_dict[key]
        data_key = proto_data_key_list[elem_type]
        proto_index = self.feed_names_to_idx_[key]
        name = self.feed_real_names[proto_index]
        alias_name = key

        # feed_dict[key] 可以是np.ndarray
        # 也可以是list或tuple
        # 当np.ndarray需要处理为list
        if isinstance(feed_dict[key], np.ndarray):
            shape_lst = []
            # 0维numpy 需要在外层再加一个[]
            if feed_dict[key].ndim == 0:
                data_value = [feed_dict[key].tolist()]
                shape_lst.append(1)
            else:
                shape_lst.extend(list(feed_dict[key].shape))
                shape = shape_lst
                data_value = feed_dict[key].flatten().tolist()
            # 当Batch为False，shape字段前插一个1,表示batch维
            # 当Batch为True,则直接使用numpy.shape作为batch维度
            if batch == False:
                shape.insert(0, 1)

            # 当是list或tuple时，需要把多层嵌套展开
        elif isinstance(feed_dict[key], (list, tuple)):
            # 当Batch为False，shape字段前插一个1,表示batch维
            # 当Batch为True, 由于list并不像numpy那样规整，所以
            # 无法获取shape，此时取第一维度作为Batch维度.
            # 插入到feedVar.shape前面.
            if batch == False:
                shape.insert(0, 1)
            else:
                shape.insert(0, len(feed_dict[key]))
            feed_dict[key] = [x for x in list_flatten(feed_dict[key])]
            data_value = feed_dict[key]
        else:
            # 输入可能是单个的str或int值等
            # 此时先统一处理为一个list
            # 由于输入比较特殊，shape保持原feedvar中不变
            data_value = []
            if isinstance(feed_dict[key], (str, bytes)):
                if self.feed_types_[key] != bytes_type:
                    raise ValueError(
                        "feedvar is not string-type,feed can`t be a single string."
                    )
                if isinstance(feed_dict[key], bytes):
                    feed_dict[key] = feed_dict[key].decode()
            else:
                if self.feed_types_[key] == bytes_type:
                    raise ValueError(
                        "feedvar is string-type,feed can`t be a single int or others."
                    )
            data_value.append(feed_dict[key])
        # 如果不压缩，那么不需要统计数据量。
        if self.try_request_gzip:
            self.total_data_number = self.total_data_number + data_bytes_number(
                data_value)
        tensor_dict = {}
        tensor_dict["data_key"] = data_key
        tensor_dict["data_value"] = data_value
        tensor_dict["shape"] = shape
        tensor_dict["elem_type"] = elem_type
        tensor_dict["name"] = name
        tensor_dict["alias_name"] = alias_name
        if len(lod) > 0:
            tensor_dict["lod"] = lod
        return tensor_dict

    # feed结构必须为dict、List、tuple、string
    # feed中数据支持Numpy、list、tuple、以及基本类型
    # fetch默认是从模型的配置文件中获取全部的fetch_var
    def predict(self,
                feed=None,
                fetch=None,
                batch=False,
                need_variant_tag=False,
                log_id=0):
        if self.use_grpc_client:
            return self.grpc_client_predict(feed, fetch, batch,
                                            need_variant_tag, log_id)
        else:
            return self.http_client_predict(feed, fetch, batch,
                                            need_variant_tag, log_id)

    def http_client_predict(self,
                            feed=None,
                            fetch=None,
                            batch=False,
                            need_variant_tag=False,
                            log_id=0):

        feed_dict = self.get_feedvar_dict(feed)
        fetch_list = self.get_legal_fetch(fetch)
        postData = ''

        if self.http_proto == True:
            postData = self.process_proto_data(feed_dict, fetch_list, batch,
                                               log_id).SerializeToString()

        else:
            postData = self.process_json_data(feed_dict, fetch_list, batch,
                                              log_id)

        web_url = self.http_s + self.ip + ":" + self.server_port + self.service_name
        # 当数据区长度大于512字节时才压缩.
        self.headers.pop("Content-Encoding", "nokey")
        try:
            if self.try_request_gzip and self.total_data_number > 512:

                if self.http_proto:
                    postData = gzip.compress(postData)
                else:
                    postData = gzip.compress(bytes(postData, 'utf-8'))
                self.headers["Content-Encoding"] = "gzip"
            if self.try_response_gzip:
                self.headers["Accept-encoding"] = "gzip"
        # 压缩异常，使用原始数据
        except:
            print("compress error, we will use the no-compress data")
            self.headers.pop("Content-Encoding", "nokey")
        # requests支持自动识别解压
        try:
            result = self.requests_session.post(
                url=web_url,
                headers=self.headers,
                data=postData,
                timeout=self.timeout_ms / 1000,
                verify=False)
            result.raise_for_status()
        except:
            print("http post error")
            return None
        else:
            if result == None:
                return None
            if result.status_code == 200:
                if result.headers["Content-Type"] == 'application/proto':
                    response = general_model_service_pb2.Response()
                    response.ParseFromString(result.content)
                    return response
                else:
                    return result.json()
            return result

    def grpc_client_predict(self,
                            feed=None,
                            fetch=None,
                            batch=False,
                            need_variant_tag=False,
                            log_id=0):

        feed_dict = self.get_feedvar_dict(feed)
        fetch_list = self.get_legal_fetch(fetch)

        postData = self.process_proto_data(feed_dict, fetch_list, batch, log_id)

        try:
            resp = self.stub_.inference(
                postData, timeout=self.timeout_ms / 1000)
        except:
            print("Grpc inference error occur")
            return None
        else:
            return resp

    def init_grpc_stub(self):
        # https://github.com/tensorflow/serving/issues/1382
        options = [('grpc.max_receive_message_length', self.max_body_size),
                   ('grpc.max_send_message_length', self.max_body_size)]

        endpoints = [self.ip + ":" + self.server_port]
        g_endpoint = 'ipv4:{}'.format(','.join(endpoints))
        self.channel_ = grpc.insecure_channel(g_endpoint, options=options)
        self.stub_ = general_model_service_pb2_grpc.GeneralModelServiceStub(
            self.channel_)

    def __del__(self):
        self.requests_session.close()
        self.channel_.close()
