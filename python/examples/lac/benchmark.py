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

import sys
import time
import requests
from paddle_serving_app.reader import LACReader
from paddle_serving_client import Client
from paddle_serving_client.utils import MultiThreadRunner
from paddle_serving_client.utils import benchmark_args

args = benchmark_args()


def single_func(idx, resource):
    reader = LACReader()
    start = time.time()
    if args.request == "rpc":
        client = Client()
        client.load_client_config(args.model)
        client.connect([args.endpoint])
        fin = open("jieba_test.txt")
        for line in fin:
            feed_data = reader.process(line)
            fetch_map = client.predict(
                feed={"words": feed_data}, fetch=["crf_decode"])
    elif args.request == "http":
        fin = open("jieba_test.txt")
        for line in fin:
            req_data = {"words": line.strip(), "fetch": ["crf_decode"]}
            r = requests.post(
                "http://{}/lac/prediction".format(args.endpoint),
                data={"words": line.strip(),
                      "fetch": ["crf_decode"]})
    end = time.time()
    return [[end - start]]


multi_thread_runner = MultiThreadRunner()
result = multi_thread_runner.run(single_func, args.thread, {})
print(result)
