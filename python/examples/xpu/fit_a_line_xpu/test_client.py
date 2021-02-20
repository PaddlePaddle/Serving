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

from paddle_serving_client import Client
import sys
import numpy as np

client = Client()
client.load_client_config(sys.argv[1])
client.connect(["127.0.0.1:9393"])

import paddle
test_reader = paddle.batch(
    paddle.reader.shuffle(
        paddle.dataset.uci_housing.test(), buf_size=500),
    batch_size=1)

for data in test_reader():
    new_data = np.zeros((1, 1, 13)).astype("float32")
    new_data[0] = data[0][0]
    fetch_map = client.predict(
        feed={"x": new_data}, fetch=["price"], batch=True)
    print(fetch_map)
