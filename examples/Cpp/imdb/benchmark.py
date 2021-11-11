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

import os
import sys
import time
import requests
import numpy as np
from paddle_serving_app.reader.imdb_reader import IMDBDataset
from paddle_serving_client import Client
from paddle_serving_client.utils import MultiThreadRunner
from paddle_serving_client.utils import MultiThreadRunner, benchmark_args, show_latency

args = benchmark_args()


def single_func(idx, resource):
    imdb_dataset = IMDBDataset()
    imdb_dataset.load_resource("./imdb.vocab")
    dataset = []
    with open("./test_data/part-0") as fin:
        for line in fin:
            dataset.append(line.strip())
    profile_flags = False
    latency_flags = False
    if os.getenv("FLAGS_profile_client"):
        profile_flags = True
    if os.getenv("FLAGS_serving_latency"):
        latency_flags = True
        latency_list = []
    start = time.time()
    if args.request == "rpc":
        client = Client()
        client.load_client_config(args.model)
        client.connect([args.endpoint])
        for i in range(1000):
            if args.batch_size >= 1:
                feed_batch = []
                feed = {"words": [], "words.lod": [0]}
                for bi in range(args.batch_size):
                    word_ids, label = imdb_dataset.get_words_and_label(dataset[
                        bi])
                    feed["words.lod"].append(feed["words.lod"][-1] + len(
                        word_ids))
                    feed["words"].extend(word_ids)
                feed["words"] = np.array(feed["words"]).reshape(
                    len(feed["words"]), 1)
                result = client.predict(
                    feed=feed, fetch=["prediction"], batch=True)
                if result is None:
                    raise ("predict failed.")
            else:
                print("unsupport batch size {}".format(args.batch_size))

    elif args.request == "http":
        if args.batch_size >= 1:
            feed_batch = []
            for bi in range(args.batch_size):
                feed_batch.append({"words": dataset[bi]})
            r = requests.post(
                "http://{}/imdb/prediction".format(args.endpoint),
                json={"feed": feed_batch,
                      "fetch": ["prediction"]})
            if r.status_code != 200:
                print('HTTP status code -ne 200')
                raise ("predict failed.")
        else:
            print("unsupport batch size {}".format(args.batch_size))
    end = time.time()
    return [[end - start]]


if __name__ == '__main__':
    multi_thread_runner = MultiThreadRunner()
    endpoint_list = [
        "127.0.0.1:9292", "127.0.0.1:9293", "127.0.0.1:9294", "127.0.0.1:9295"
    ]
    turns = 100
    start = time.time()
    result = multi_thread_runner.run(
        single_func, args.thread, {"endpoint": endpoint_list,
                                   "turns": turns})
    end = time.time()
    total_cost = end - start
    avg_cost = 0
    for i in range(args.thread):
        avg_cost += result[0][i]
    avg_cost = avg_cost / args.thread

    print("total cost: {}".format(total_cost))
    print("each thread cost: {}".format(avg_cost))
    print("qps: {}samples/s".format(args.batch_size * args.thread * turns /
                                    total_cost))
    if os.getenv("FLAGS_serving_latency"):
        show_latency(result[0])
