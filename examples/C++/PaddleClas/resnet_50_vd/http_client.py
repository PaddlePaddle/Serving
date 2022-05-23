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

#app
from paddle_serving_app.reader import Sequential, URL2Image, Resize
from paddle_serving_app.reader import CenterCrop, RGB2BGR, Transpose, Div, Normalize
import time

client = HttpClient()
client.load_client_config("./ResNet50_vd_client/serving_client_conf.prototxt")
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
client.connect(["127.0.0.1:9394"])

label_dict = {}
label_idx = 0
with open("imagenet.label") as fin:
    for line in fin:
        label_dict[label_idx] = line.strip()
        label_idx += 1

#preprocess
seq = Sequential([
    URL2Image(), Resize(256), CenterCrop(224), RGB2BGR(), Transpose((2, 0, 1)),
    Div(255), Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225], True)
])

start = time.time()
image_file = "https://paddle-serving.bj.bcebos.com/imagenet-example/daisy.jpg"
for i in range(1):
    img = seq(image_file)
    res = client.predict(feed={"inputs": img}, fetch=[], batch=False)

    if res is None:
        raise ValueError("predict error")

    if res.err_no != 0:
        raise ValueError("predict error. Response : {}".format(res))

    max_val = res.outputs[0].tensor[0].float_data[0]
    max_idx = 0
    for one_data in enumerate(res.outputs[0].tensor[0].float_data):
        if one_data[1] > max_val:
            max_val = one_data[1]
            max_idx = one_data[0]
    label = label_dict[max_idx].strip().replace(",", "")
    print("prediction: {}, probability: {}".format(label, max_val))
end = time.time()
print(end - start)
