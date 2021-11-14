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
from paddle_serving_client.utils import MultiThreadRunner
from paddle_serving_client.utils import benchmark_args, show_latency
import time
import paddle
import sys
import requests

args = benchmark_args()


def single_func(idx, resource):
    train_reader = paddle.batch(
        paddle.reader.shuffle(
            paddle.dataset.uci_housing.train(), buf_size=500),
        batch_size=1)
    total_number = sum(1 for _ in train_reader())
    latency_list = []

    if args.request == "rpc":
        client = Client()
        client.load_client_config(args.model)
        client.connect([args.endpoint])
        start = time.time()
        for data in train_reader():
            l_start = time.time()
            fetch_map = client.predict(feed={"x": data[0][0]}, fetch=["price"])
            l_end = time.time()
            latency_list.append(l_end * 1000 - l_start * 1000)
        end = time.time()
        return [[end - start], latency_list, [total_number]]
    elif args.request == "http":
        train_reader = paddle.batch(
            paddle.reader.shuffle(
                paddle.dataset.uci_housing.train(), buf_size=500),
            batch_size=1)
        start = time.time()
        for data in train_reader():
            l_start = time.time()
            r = requests.post(
                'http://{}/uci/prediction'.format(args.endpoint),
                data={"x": data[0]})
            l_end = time.time()
            latency_list.append(l_end * 1000 - l_start * 1000)
        end = time.time()
        return [[end - start], latency_list, [total_number]]


start = time.time()
multi_thread_runner = MultiThreadRunner()
result = multi_thread_runner.run(single_func, args.thread, {})
end = time.time()
total_cost = end - start
avg_cost = 0
for i in range(args.thread):
    avg_cost += result[0][i]
avg_cost = avg_cost / args.thread

print("total cost: {}s".format(total_cost))
print("each thread cost: {}s. ".format(avg_cost))
print("qps: {}samples/s".format(args.batch_size * args.thread / total_cost))
show_latency(result[1])
