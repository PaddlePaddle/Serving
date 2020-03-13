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
from image_reader import ImageReader
from paddle_serving_client import Client
from paddle_serving_client.utils import MultiThreadRunner
from paddle_serving_client.utils import benchmark_args
import time

args = benchmark_args()


def single_func(idx, resource):
    if args.request == "rpc":
        reader = ImageReader()
        fetch = ["score"]
        client = Client()
        client.load_client_config(args.model)
        client.connect([resource["endpoint"][idx % 4]])

        start = time.time()
        for i in range(1000):
            with open("./data/n01440764_10026.JPEG") as f:
                img = f.read()
            img = reader.process_image(img).reshape(-1)
            fetch_map = client.predict(feed={"image": img}, fetch=["score"])
        end = time.time()
        return [[end - start]]
    return [[end - start]]


if __name__ == "__main__":
    multi_thread_runner = MultiThreadRunner()
    endpoint_list = []
    card_num = 4
    for i in range(args.thread):
        endpoint_list.append("127.0.0.1:{}".format(9295 + i % card_num))
    result = multi_thread_runner.run(single_func, args.thread,
                                     {"endpoint": endpoint_list})
    print(result)
