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

from paddle_serving_client import Client
from paddle_serving_client.utils import MultiThreadRunner
import paddle
import numpy as np


def single_func(idx, resource):
    client = Client()
    client.load_client_config(
        "./uci_housing_client/serving_client_conf.prototxt")
    client.connect(["127.0.0.1:9293", "127.0.0.1:9292"])
    x = [
        0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584,
        0.6283, 0.4919, 0.1856, 0.0795, -0.0332
    ]
    x = np.array(x)
    for i in range(1000):
        fetch_map = client.predict(feed={"x": x}, fetch=["price"])
        if fetch_map is None:
            return [[None]]
    return [[0]]


multi_thread_runner = MultiThreadRunner()
thread_num = 4
result = multi_thread_runner.run(single_func, thread_num, {})
if None in result[0]:
    exit(1)
