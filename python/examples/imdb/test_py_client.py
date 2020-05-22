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
import general_python_service_pb2
import general_python_service_pb2_grpc
import numpy as np

channel = grpc.insecure_channel('localhost:8080')
stub = general_python_service_pb2_grpc.GeneralPythonServiceStub(channel)
req = general_python_service_pb2.Request()
"""
# line = "i am very sad | 0"
word_ids = np.array([8, 233, 52, 601], dtype='int64')
# word_ids = np.array([8, 233, 52, 601])
print(word_ids)
data = np.ndarray.tobytes(word_ids)
print(data)
# xx = np.frombuffer(data)
xx = np.frombuffer(data, dtype='int64')
print (xx)
req.feed_var_names.append("words")
req.feed_insts.append(data)
"""
x = np.array(
    [
        0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584,
        0.6283, 0.4919, 0.1856, 0.0795, -0.0332
    ],
    dtype='float')
data = np.ndarray.tobytes(x)
req.feed_var_names.append("x")
req.feed_insts.append(data)

for i in range(100):
    resp = stub.inference(req)
    for idx, name in enumerate(resp.fetch_var_names):
        print('{}: {}'.format(
            name, np.frombuffer(
                resp.fetch_insts[idx], dtype='float')))
