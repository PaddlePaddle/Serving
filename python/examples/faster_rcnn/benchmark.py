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
import json
import requests
from paddle_serving_client import Client
from paddle_serving_client.utils import MultiThreadRunner
from paddle_serving_client.utils import benchmark_args, show_latency
from paddle_serving_app.reader import ChineseBertReader

from paddle_serving_app.reader import *
import numpy as np

args = benchmark_args()


def single_func(idx, resource):
    img = "./000000570688.jpg"
    profile_flags = False
    latency_flags = False
    if os.getenv("FLAGS_profile_client"):
        profile_flags = True
    if os.getenv("FLAGS_serving_latency"):
        latency_flags = True
        latency_list = []

    if args.request == "rpc":
        preprocess = Sequential([
            File2Image(), BGR2RGB(), Div(255.0),
            Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225], False),
            Resize(640, 640), Transpose((2, 0, 1))
        ])

        postprocess = RCNNPostprocess("label_list.txt", "output")
        client = Client()

        client.load_client_config(args.model)
        client.connect([resource["endpoint"][idx % len(resource["endpoint"])]])

        start = time.time()
        for i in range(turns):
            if args.batch_size >= 1:
                l_start = time.time()
                feed_batch = []
                b_start = time.time()
                im = preprocess(img)
                for bi in range(args.batch_size):
                    print("1111batch")
                    print(bi)
                    feed_batch.append({
                        "image": im,
                        "im_info": np.array(list(im.shape[1:]) + [1.0]),
                        "im_shape": np.array(list(im.shape[1:]) + [1.0])
                    })
            # im = preprocess(img)
                b_end = time.time()

                if profile_flags:
                    sys.stderr.write(
                        "PROFILE\tpid:{}\tbert_pre_0:{} bert_pre_1:{}\n".format(
                            os.getpid(),
                            int(round(b_start * 1000000)),
                            int(round(b_end * 1000000))))
                #result = client.predict(feed=feed_batch, fetch=fetch)
                fetch_map = client.predict(
                    feed=feed_batch, fetch=["multiclass_nms"])
                fetch_map["image"] = img
                postprocess(fetch_map)

                l_end = time.time()
                if latency_flags:
                    latency_list.append(l_end * 1000 - l_start * 1000)
            else:
                print("unsupport batch size {}".format(args.batch_size))
    else:
        raise ValueError("not implemented {} request".format(args.request))
    end = time.time()
    if latency_flags:
        return [[end - start], latency_list]
    else:
        return [[end - start]]


if __name__ == '__main__':
    multi_thread_runner = MultiThreadRunner()
    endpoint_list = ["127.0.0.1:7777"]
    turns = 10
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

    print("total cost: {}s".format(total_cost))
    print("each thread cost: {}s. ".format(avg_cost))
    print("qps: {}samples/s".format(args.batch_size * args.thread * turns /
                                    total_cost))
    if os.getenv("FLAGS_serving_latency"):
        show_latency(result[1])
