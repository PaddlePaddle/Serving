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

channel = grpc.insecure_channel('127.0.0.1:8080')
stub = general_python_service_pb2_grpc.GeneralPythonServiceStub(channel)

# line = "i am very sad | 0"
word_ids = np.array([8, 233, 52, 601])

req = general_python_service_pb2.Request()
req.feed_var_names.append("words")
req.feed_insts.append(np.ndarray.tobytes(word_ids))

resp = stub.inference(req)
print(resp)
