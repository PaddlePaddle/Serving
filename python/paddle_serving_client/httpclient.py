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

#param 'type'(which is in feed_var or fetch_var) = 0 means dataType is int64
#param 'type'(which is in feed_var or fetch_var) = 1 means dataType is float32
#param 'type'(which is in feed_var or fetch_var) = 2 means dataType is int32
#param 'type'(which is in feed_var or fetch_var) = 3 means dataType is string(also called bytes in proto)
int64_type = 0
float32_type = 1
int32_type = 2
bytes_type = 3
# this is corresponding to the proto
proto_data_key_list = ["int64_data", "float_data", "int_data", "data"]


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


class HttpClient(object):
    def __init__(self,
                 ip="0.0.0.0",
                 port="9393",
                 service_name="/GeneralModelService/inference"):
        self.feed_names_ = []
        self.feed_real_names = []
        self.fetch_names_ = []
        self.feed_shapes_ = {}
        self.feed_types_ = {}
        self.feed_names_to_idx_ = {}
        self.http_timeout_ms = 200000
        self.ip = ip
        self.port = port
        self.server_port = port
        self.service_name = service_name
        self.key = None
        self.try_request_gzip = False
        self.try_response_gzip = False

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

    def set_http_timeout_ms(self, http_timeout_ms):
        if not isinstance(http_timeout_ms, int):
            raise ValueError("http_timeout_ms must be int type.")
        else:
            self.http_timeout_ms = http_timeout_ms

    def set_request_compress(self, try_request_gzip):
        self.try_request_gzip = try_request_gzip

    def set_response_compress(self, try_response_gzip):
        self.try_response_gzip = try_response_gzip

    # use_key is the function of encryption.
    def use_key(self, key_filename):
        with open(key_filename, "rb") as f:
            self.key = f.read()
            self.get_serving_port()

    def get_serving_port(self):
        encrypt_url = "http://" + str(self.ip) + ":" + str(self.port)
        if self.key is not None:
            req = json.dumps({"key": base64.b64encode(self.key).decode()})
        else:
            req = json.dumps({})
        r = requests.post(encrypt_url, req)
        result = r.json()
        print(result)
        if "endpoint_list" not in result:
            raise ValueError("server not ready")
        else:
            self.server_port = str(result["endpoint_list"][0])
            print("rpc port is ", self.server_port)

    def get_feed_names(self):
        return self.feed_names_

    def get_fetch_names(self):
        return self.fetch_names_

    # feed 支持Numpy类型，以及直接List、tuple
    # 不支持str类型，因为proto中为repeated.
    def predict(self,
                feed=None,
                fetch=None,
                batch=False,
                need_variant_tag=False,
                log_id=0):
        if feed is None or fetch is None:
            raise ValueError("You should specify feed and fetch for prediction")

        fetch_list = []
        if isinstance(fetch, str):
            fetch_list = [fetch]
        elif isinstance(fetch, (list, tuple)):
            fetch_list = fetch
        else:
            raise ValueError("Fetch only accepts string and list of string")

        feed_batch = []
        if isinstance(feed, dict):
            feed_batch.append(feed)
        elif isinstance(feed, (list, str, tuple)):
            # if input is a list or str or tuple, and the number of feed_var is 1.
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

        fetch_names = []
        for key in fetch_list:
            if key in self.fetch_names_:
                fetch_names.append(key)

        if len(fetch_names) == 0:
            raise ValueError(
                "Fetch names should not be empty or out of saved fetch list.")
            return {}

        feed_i = feed_batch[0]

        Request = {}
        Request["fetch_var_names"] = fetch_list
        Request["log_id"] = int(log_id)
        Request["tensor"] = []
        index = 0
        total_data_number = 0
        for key in feed_i:
            if ".lod" not in key and key not in self.feed_names_:
                raise ValueError("Wrong feed name: {}.".format(key))
            if ".lod" in key:
                continue

            Request["tensor"].append('')
            Request["tensor"][index] = {}
            lod = []
            if "{}.lod".format(key) in feed_i:
                lod = feed_i["{}.lod".format(key)]
            shape = self.feed_shapes_[key].copy()
            elem_type = self.feed_types_[key]
            data_value = feed_i[key]
            data_key = proto_data_key_list[elem_type]

            # feed_i[key] 可以是np.ndarray
            # 也可以是list或tuple
            # 当np.ndarray需要处理为list
            if isinstance(feed_i[key], np.ndarray):
                shape_lst = []
                # 0维numpy 需要在外层再加一个[]
                if feed_i[key].ndim == 0:
                    data_value = [feed_i[key].tolist()]
                    shape_lst.append(1)
                else:
                    shape_lst.extend(list(feed_i[key].shape))
                    shape = shape_lst
                    data_value = feed_i[key].flatten().tolist()
                # 当Batch为False，shape字段前插一个1,表示batch维
                # 当Batch为True,则直接使用numpy.shape作为batch维度
                if batch == False:
                    shape.insert(0, 1)

                # 当是list或tuple时，需要把多层嵌套展开
            elif isinstance(feed_i[key], (list, tuple)):
                # 当Batch为False，shape字段前插一个1,表示batch维
                # 当Batch为True, 由于list并不像numpy那样规整，所以
                # 无法获取shape，此时取第一维度作为Batch维度.
                # 插入到feedVar.shape前面.
                if batch == False:
                    shape.insert(0, 1)
                else:
                    shape.insert(0, len(feed_i[key]))
                feed_i[key] = [x for x in list_flatten(feed_i[key])]
                data_value = feed_i[key]
            else:
                # 输入可能是单个的str或int值等
                # 此时先统一处理为一个list
                # 由于输入比较特殊，shape保持原feedvar中不变
                data_value = []
                data_value.append(feed_i[key])
                if isinstance(feed_i[key], str):
                    if self.feed_types_[key] != bytes_type:
                        raise ValueError(
                            "feedvar is not string-type,feed can`t be a single string."
                        )
                else:
                    if self.feed_types_[key] == bytes_type:
                        raise ValueError(
                            "feedvar is string-type,feed, feed can`t be a single int or others."
                        )

            total_data_number = total_data_number + data_bytes_number(
                data_value)
            Request["tensor"][index]["elem_type"] = elem_type
            Request["tensor"][index]["shape"] = shape
            Request["tensor"][index][data_key] = data_value
            proto_index = self.feed_names_to_idx_[key]
            Request["tensor"][index]["name"] = self.feed_real_names[proto_index]
            Request["tensor"][index]["alias_name"] = key
            if len(lod) > 0:
                Request["tensor"][index]["lod"] = lod
            index = index + 1

        result = None
        # request
        web_url = "http://" + self.ip + ":" + self.server_port + self.service_name
        postData = json.dumps(Request)
        headers = {}
        # 当数据区长度大于512字节时才压缩.
        if self.try_request_gzip and total_data_number > 512:
            postData = gzip.compress(bytes(postData, 'utf-8'))
            headers["Content-Encoding"] = "gzip"
        if self.try_response_gzip:
            headers["Accept-encoding"] = "gzip"
        # requests支持自动识别解压
        result = requests.post(url=web_url, headers=headers, data=postData)

        if result == None:
            return None
        if result.status_code == 200:
            return result.json()
        return result
