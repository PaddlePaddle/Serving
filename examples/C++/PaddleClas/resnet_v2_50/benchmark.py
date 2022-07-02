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
import numpy as np
from paddle_serving_client import Client
from paddle_serving_client.utils import MultiThreadRunner
from paddle_serving_client.utils import benchmark_args, show_latency
from paddle_serving_app.reader import Sequential, File2Image, Resize, CenterCrop
from paddle_serving_app.reader import RGB2BGR, Transpose, Div, Normalize

args = benchmark_args()


def single_func(idx, resource):
    total_number = 0
    profile_flags = False
    latency_flags = False
    if os.getenv("FLAGS_profile_client"):
        profile_flags = True
    if os.getenv("FLAGS_serving_latency"):
        latency_flags = True
        latency_list = []

    if args.request == "rpc":
        client = Client()
        client.load_client_config(args.model)
        client.connect([resource["endpoint"][idx % len(resource["endpoint"])]])
        start = time.time()
        for i in range(turns):
            if args.batch_size >= 1:
                l_start = time.time()
                seq = Sequential([
                    File2Image(), Resize(256), CenterCrop(224), RGB2BGR(),
                    Transpose((2, 0, 1)), Div(255), Normalize(
                        [0.485, 0.456, 0.406], [0.229, 0.224, 0.225], True)
                ])
                image_file = "daisy.jpg"
                img = seq(image_file)
                feed_data = np.array(img)
                feed_data = np.expand_dims(feed_data, 0).repeat(
                    args.batch_size, axis=0)
                result = client.predict(
                    feed={"image": feed_data},
                    fetch=["save_infer_model/scale_0.tmp_0"],
                    batch=True)
                l_end = time.time()
                if latency_flags:
                    latency_list.append(l_end * 1000 - l_start * 1000)
                total_number = total_number + 1
            else:
                print("unsupport batch size {}".format(args.batch_size))

    else:
        raise ValueError("not implemented {} request".format(args.request))
    end = time.time()
    if latency_flags:
        return [[end - start], latency_list, [total_number]]
    else:
        return [[end - start]]


if __name__ == '__main__':
    multi_thread_runner = MultiThreadRunner()
    endpoint_list = ["127.0.0.1:9393"]
    turns = 1
    start = time.time()
    result = multi_thread_runner.run(
        single_func, args.thread, {"endpoint": endpoint_list,
                                   "turns": turns})
    end = time.time()
    total_cost = end - start
    total_number = 0
    avg_cost = 0
    for i in range(args.thread):
        avg_cost += result[0][i]
        total_number += result[2][i]
    avg_cost = avg_cost / args.thread

    print("total cost-include init: {}s".format(total_cost))
    print("each thread cost: {}s. ".format(avg_cost))
    print("qps: {}samples/s".format(args.batch_size * total_number / (
        avg_cost * args.thread)))
    print("qps(request): {}samples/s".format(total_number / (avg_cost *
                                                             args.thread)))
    print("total count: {} ".format(total_number))
    if os.getenv("FLAGS_serving_latency"):
        show_latency(result[1])
