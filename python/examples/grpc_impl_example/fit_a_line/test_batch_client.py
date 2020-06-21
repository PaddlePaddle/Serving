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

from paddle_serving_client import MultiLangClient as Client
import paddle

client = Client()
client.connect(["127.0.0.1:9393"])

batch_size = 2
test_reader = paddle.batch(
    paddle.reader.shuffle(
        paddle.dataset.uci_housing.test(), buf_size=500),
    batch_size=batch_size)

for data in test_reader():
    batch_feed = [{"x": x[0]} for x in data]
    try:
        fetch_map = client.predict(feed=batch_feed, fetch=["price"])
    except grpc.RpcError as e:
        status_code = e.code()
        if grpc.StatusCode.DEADLINE_EXCEEDED == status_code:
            print('timeout')
    else:
        print(fetch_map)
