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
    profile_flags = False
    if os.getenv("FLAGS_profile_client"):
        profile_flags = True
    if args.request == "rpc":
        reader = BertReader(vocab_file="vocab.txt", max_seq_len=20)
        fetch = ["pooled_output"]
        client = Client()
        client.load_client_config(args.model)
        client.connect([resource["endpoint"][idx % len(resource["endpoint"])]])
        start = time.time()
        for i in range(turns):
            if args.batch_size >= 1:
                feed_batch = []
                b_start = time.time()
                for bi in range(args.batch_size):
                    feed_batch.append(reader.process(dataset[bi]))
                b_end = time.time()
                if profile_flags:
                    sys.stderr.write(
                        "PROFILE\tpid:{}\tbert_pre_0:{} bert_pre_1:{}\n".format(
                            os.getpid(),
                            int(round(b_start * 1000000)),
                            int(round(b_end * 1000000))))
                result = client.predict(feed=feed_batch, fetch=fetch)
            else:
                print("unsupport batch size {}".format(args.batch_size))

    elif args.request == "http":
        raise ("not implemented")
    end = time.time()
    return [[end - start]]


if __name__ == '__main__':
    multi_thread_runner = MultiThreadRunner()
    endpoint_list = [
        "127.0.0.1:9292", "127.0.0.1:9293", "127.0.0.1:9294", "127.0.0.1:9295"
    ]
    turns = 1000
    start = time.time()
    result = multi_thread_runner.run(
        single_func, args.thread, {"endpoint": endpoint_list,
                                   "turns": turns})
    avg_cost = 0
    for i in range(args.thread):
        avg_cost += result[0][i]
    avg_cost = avg_cost / args.thread
    end = time.time()
    total_cost = end - start
    print("total cost :{} s".format(total_cost))
    print("each thread cost :{} s. ".format(avg_cost))
    print("qps :{} samples/s".format(args.batch_size * args.thread * turns /
                                     total_cost))
