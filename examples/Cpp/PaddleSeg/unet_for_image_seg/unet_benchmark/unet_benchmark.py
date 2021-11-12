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
"""
  unet bench mark script
  20201130 first edition by cg82616424
"""
from __future__ import unicode_literals, absolute_import
import os
import time
import json
import requests
from paddle_serving_client import Client
from paddle_serving_client.utils import MultiThreadRunner
from paddle_serving_client.utils import benchmark_args, show_latency
from paddle_serving_app.reader import Sequential, File2Image, Resize, Transpose, BGR2RGB, SegPostprocess
args = benchmark_args()


def get_img_names(path):
    """
    Brief:
        get img files(jpg) under this path
        if any exception happened return None
    Args:
        path (string): image file path
    Returns:
        list: images names under this folder
    """
    if not os.path.exists(path):
        return None
    if not os.path.isdir(path):
        return None
    list_name = []
    for f_handler in os.listdir(path):
        file_path = os.path.join(path, f_handler)
        if os.path.isdir(file_path):
            continue
        else:
            if not file_path.endswith(".jpeg") and not file_path.endswith(
                    ".jpg"):
                continue
            list_name.append(file_path)
    return list_name


def preprocess_img(img_list):
    """
    Brief:
        prepare img data for benchmark
    Args:
        img_list(list): list for img file path
    Returns:
        image content binary list after preprocess
    """
    preprocess = Sequential([File2Image(), Resize((512, 512))])
    result_list = []
    for img in img_list:
        img_tmp = preprocess(img)
        result_list.append(img_tmp)
    return result_list


def benckmark_worker(idx, resource):
    """
    Brief:
        benchmark single worker for unet
    Args:
        idx(int): worker idx ,use idx to select backend unet service
        resource(dict): unet serving endpoint dict 
    Returns:
        latency
    TODO:
        http benckmarks
    """
    profile_flags = False
    latency_flags = False
    postprocess = SegPostprocess(2)
    if os.getenv("FLAGS_profile_client"):
        profile_flags = True
    if os.getenv("FLAGS_serving_latency"):
        latency_flags = True
        latency_list = []
    client_handler = Client()
    client_handler.load_client_config(args.model)
    client_handler.connect(
        [resource["endpoint"][idx % len(resource["endpoint"])]])
    start = time.time()
    turns = resource["turns"]
    img_list = resource["img_list"]
    for i in range(turns):
        if args.batch_size >= 1:
            l_start = time.time()
            feed_batch = []
            b_start = time.time()
            for bi in range(args.batch_size):
                feed_batch.append({"image": img_list[bi]})
            b_end = time.time()
            if profile_flags:
                sys.stderr.write(
                    "PROFILE\tpid:{}\tunt_pre_0:{} unet_pre_1:{}\n".format(
                        os.getpid(),
                        int(round(b_start * 1000000)),
                        int(round(b_end * 1000000))))
            result = client_handler.predict(
                feed={"image": img_list[bi]}, fetch=["output"])
            #result["filename"] = "./img_data/N0060.jpg" % (os.getpid(), idx, time.time())
            #postprocess(result) # if you  want to measure post process time, you have to uncomment this line
            l_end = time.time()
            if latency_flags:
                latency_list.append(l_end * 1000 - l_start * 1000)
        else:
            print("unsupport batch size {}".format(args.batch_size))
    end = time.time()
    if latency_flags:
        return [[end - start], latency_list]
    else:
        return [[end - start]]


if __name__ == '__main__':
    """
    usage: 
    """
    img_file_list = get_img_names("./img_data")
    img_content_list = preprocess_img(img_file_list)
    multi_thread_runner = MultiThreadRunner()
    endpoint_list = ["127.0.0.1:9494"]
    turns = 1
    start = time.time()
    result = multi_thread_runner.run(benckmark_worker, args.thread, {
        "endpoint": endpoint_list,
        "turns": turns,
        "img_list": img_content_list
    })
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
