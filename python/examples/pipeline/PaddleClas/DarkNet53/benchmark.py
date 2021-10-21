# Copyright (c) 2021 PaddlePaddle Authors. All Rights Reserved.
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
import os
import base64
import yaml
import requests
import time
import json

from paddle_serving_server.pipeline import PipelineClient
import numpy as np
from paddle_serving_client.utils import MultiThreadRunner
from paddle_serving_client.utils import benchmark_args, show_latency


def parse_benchmark(filein, fileout):
    with open(filein, "r") as fin:
        res = yaml.load(fin, yaml.FullLoader)
        del_list = []
        for key in res["DAG"].keys():
            if "call" in key:
                del_list.append(key)
        for key in del_list:
            del res["DAG"][key]
    with open(fileout, "w") as fout:
        yaml.dump(res, fout, default_flow_style=False)


def gen_yml(device, gpu_id):
    fin = open("config.yml", "r")
    config = yaml.load(fin, yaml.FullLoader)
    fin.close()
    config["dag"]["tracer"] = {"interval_s": 10}
    if device == "gpu":
        config["op"]["imagenet"]["local_service_conf"]["device_type"] = 1
        config["op"]["imagenet"]["local_service_conf"]["devices"] = gpu_id
    else:
        config["op"]["imagenet"]["local_service_conf"]["device_type"] = 0
    with open("config2.yml", "w") as fout:
        yaml.dump(config, fout, default_flow_style=False)


def cv2_to_base64(image):
    return base64.b64encode(image).decode('utf8')


def run_http(idx, batch_size):
    print("start thread ({})".format(idx))
    url = "http://127.0.0.1:18080/imagenet/prediction"
    start = time.time()

    with open(os.path.join(".", "daisy.jpg"), 'rb') as file:
        image_data1 = file.read()
    image = cv2_to_base64(image_data1)
    keys, values = [], []
    for i in range(batch_size):
        keys.append("image_{}".format(i))
        values.append(image)
    data = {"key": keys, "value": values}
    latency_list = []
    start_time = time.time()
    total_num = 0
    while True:
        l_start = time.time()
        r = requests.post(url=url, data=json.dumps(data))
        print(r.json())
        l_end = time.time()
        latency_list.append(l_end * 1000 - l_start * 1000)
        total_num += 1
        if time.time() - start_time > 20:
            break
    end = time.time()
    return [[end - start], latency_list, [total_num]]


def multithread_http(thread, batch_size):
    multi_thread_runner = MultiThreadRunner()
    start = time.time()
    result = multi_thread_runner.run(run_http, thread, batch_size)
    end = time.time()
    total_cost = end - start
    avg_cost = 0
    total_number = 0
    for i in range(thread):
        avg_cost += result[0][i]
        total_number += result[2][i]
    avg_cost = avg_cost / thread
    print("Total cost: {}s".format(total_cost))
    print("Each thread cost: {}s. ".format(avg_cost))
    print("Total count: {}. ".format(total_number))
    print("AVG_QPS: {} samples/s".format(batch_size * total_number /
                                         total_cost))
    show_latency(result[1])


def run_rpc(thread, batch_size):
    client = PipelineClient()
    client.connect(['127.0.0.1:18080'])
    start = time.time()
    test_img_dir = "imgs/"
    for img_file in os.listdir(test_img_dir):
        with open(os.path.join(test_img_dir, img_file), 'rb') as file:
            image_data = file.read()
        image = cv2_to_base64(image_data)
        start_time = time.time()
        while True:
            ret = client.predict(feed_dict={"image": image}, fetch=["res"])
            if time.time() - start_time > 10:
                break
    end = time.time()
    return [[end - start]]


def multithread_rpc(thraed, batch_size):
    multi_thread_runner = MultiThreadRunner()
    result = multi_thread_runner.run(run_rpc, thread, batch_size)


if __name__ == "__main__":
    if sys.argv[1] == "yaml":
        mode = sys.argv[2]  # brpc/  local predictor
        thread = int(sys.argv[3])
        device = sys.argv[4]
        if device == "gpu":
            gpu_id = sys.argv[5]
        else:
            gpu_id = None
        gen_yml(device, gpu_id)
    elif sys.argv[1] == "run":
        mode = sys.argv[2]  # http/ rpc
        thread = int(sys.argv[3])
        batch_size = int(sys.argv[4])
        if mode == "http":
            multithread_http(thread, batch_size)
        elif mode == "rpc":
            multithread_rpc(thread, batch_size)
    elif sys.argv[1] == "dump":
        filein = sys.argv[2]
        fileout = sys.argv[3]
        parse_benchmark(filein, fileout)
