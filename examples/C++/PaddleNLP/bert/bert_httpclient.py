# coding:utf-8
# pylint: disable=doc-string-missing
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

import sys
from paddle_serving_client import HttpClient
from paddle_serving_client.utils import benchmark_args
from paddle_serving_app.reader import ChineseBertReader
import numpy as np
args = benchmark_args()

reader = ChineseBertReader({"max_seq_len": 128})
fetch = ["pooled_output"]
endpoint_list = ['127.0.0.1:9292']
client = HttpClient()
client.load_client_config(args.model)
''' 
if you want use GRPC-client, set_use_grpc_client(True)
or you can directly use client.grpc_client_predict(...)
as for HTTP-client,set_use_grpc_client(False)(which is default)
or you can directly use client.http_client_predict(...)
'''
#client.set_use_grpc_client(True)
'''
if you want to enable Encrypt Module,uncommenting the following line
'''
#client.use_key("./key")
'''
if you want to compress,uncommenting the following line
'''
#client.set_response_compress(True)
#client.set_request_compress(True)
'''
we recommend use Proto data format in HTTP-body, set True(which is default)
if you want use JSON data format in HTTP-body, set False
'''
#client.set_http_proto(True)
client.connect(endpoint_list)

for line in sys.stdin:
    feed_dict = reader.process(line)
    for key in feed_dict.keys():
        feed_dict[key] = np.array(feed_dict[key]).reshape((128, 1))
    #print(feed_dict)
    result = client.predict(feed=feed_dict, fetch=fetch, batch=False)
print(result)
