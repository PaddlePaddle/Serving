# -*- coding: utf-8 -*-
#
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

from __future__ import unicode_literals, absolute_import
import os
import sys
import time
from paddle_serving_client import Client
from paddle_serving_client.utils import MultiThreadRunner
from paddle_serving_client.utils import benchmark_args
from batching import pad_batch_data
import tokenization
import requests
import json
from bert_reader import BertReader

args = benchmark_args()

batch_size = 24


def single_func(idx, resource):
    fin = open("data-c.txt")
    if args.request == "rpc":
        reader = BertReader(vocab_file="vocab.txt", max_seq_len=128)
        fetch = ["pooled_output"]
        client = Client()
        client.load_client_config(args.model)
        client.connect([resource["endpoint"][idx % 4]])

        start = time.time()
        idx = 0
        batch_data = []
        for line in fin:
            feed_dict = reader.process(line)
            batch_data.append(feed_dict)
            idx += 1
            if idx % batch_size == 0:
                result = client.batch_predict(
                    feed_batch=batch_data, fetch=fetch)
                batch_data = []
        end = time.time()
    elif args.request == "http":
        header = {"Content-Type": "application/json"}
        for line in fin:
            dict_data = {"words": line, "fetch": ["pooled_output"]}
            r = requests.post(
                'http://{}/bert/prediction'.format(resource["endpoint"][0]),
                data=json.dumps(dict_data),
                headers=header)
        end = time.time()
    return [[end - start]]


if __name__ == '__main__':
    multi_thread_runner = MultiThreadRunner()
    endpoint_list = []
    card_num = 4
    for i in range(args.thread):
        endpoint_list.append("127.0.0.1:{}".format(9494 + i % card_num))
    print(endpoint_list)
    result = multi_thread_runner.run(single_func, args.thread,
                                     {"endpoint": endpoint_list})
    print(result)
