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


def single_func(idx, resource):
    fin = open("data-c.txt")
    dataset = []
    for line in fin:
        dataset.append(line.strip())
    if args.request == "rpc":
        reader = BertReader(vocab_file="vocab.txt", max_seq_len=20)
        fetch = ["pooled_output"]
        client = Client()
        client.load_client_config(args.model)
        client.add_variant(
            "var1", [resource["endpoint"][idx % len(resource["endpoint"])]], 50)
        client.connect()
        feed_batch = []
        for bi in range(args.batch_size):
            feed_batch.append(reader.process(dataset[bi]))

        start = time.time()
        for i in range(1000):
            if args.batch_size >= 1:
                result = client.batch_predict(
                    feed_batch=feed_batch, fetch=fetch)
            else:
                print("unsupport batch size {}".format(args.batch_size))

    elif args.request == "http":
        raise ("no batch predict for http")
    end = time.time()
    return [[end - start]]


if __name__ == '__main__':
    multi_thread_runner = MultiThreadRunner()
    endpoint_list = [
        "127.0.0.1:9295", "127.0.0.1:9296", "127.0.0.1:9297", "127.0.0.1:9298"
    ]
    result = multi_thread_runner.run(single_func, args.thread,
                                     {"endpoint": endpoint_list})
    avg_cost = 0
    for i in range(args.thread):
        avg_cost += result[0][i]
    avg_cost = avg_cost / args.thread
    print("average total cost {} s.".format(avg_cost))
