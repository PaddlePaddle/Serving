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


def gen_yml(device):
    fin = open("config.yml", "r")
    config = yaml.load(fin, yaml.FullLoader)
    fin.close()
    config["dag"]["tracer"] = {"interval_s": 10}
    if device == "gpu":
        config["op"]["det"]["local_service_conf"]["device_type"] = 1
        config["op"]["det"]["local_service_conf"]["devices"] = "2"
        config["op"]["rec"]["local_service_conf"]["device_type"] = 1
        config["op"]["rec"]["local_service_conf"]["devices"] = "2"
    with open("config2.yml", "w") as fout:
        yaml.dump(config, fout, default_flow_style=False)


def cv2_to_base64(image):
    return base64.b64encode(image).decode('utf8')


def run_http(idx, batch_size):
    print("start thread ({})".format(idx))
    url = "http://127.0.0.1:9999/ocr/prediction"
    start = time.time()
    test_img_dir = "imgs/"
    #test_img_dir = "rctw_test/images/"
    latency_list = []
    total_number = 0
    for img_file in os.listdir(test_img_dir):
        l_start = time.time()
        with open(os.path.join(test_img_dir, img_file), 'rb') as file:
            image_data1 = file.read()
        image = cv2_to_base64(image_data1)
        data = {"key": ["image"], "value": [image]}
        #for i in range(100):
        r = requests.post(url=url, data=json.dumps(data))
        print(r.json())
        end = time.time()
        l_end = time.time()
        latency_list.append(l_end * 1000 - l_start * 1000)
        total_number = total_number + 1
    return [[end - start], latency_list, [total_number]]


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
    print("AVG QPS: {} samples/s".format(batch_size * total_number /
                                         total_cost))
    show_latency(result[1])


def run_rpc(thread, batch_size):
    client = PipelineClient()
    client.connect(['127.0.0.1:18090'])
    start = time.time()
    test_img_dir = "imgs/"
    #test_img_dir = "rctw_test/images/"
    latency_list = []
    total_number = 0
    for img_file in os.listdir(test_img_dir):
        l_start = time.time()
        with open(os.path.join(test_img_dir, img_file), 'rb') as file:
            image_data = file.read()
        image = cv2_to_base64(image_data)
        ret = client.predict(feed_dict={"image": image}, fetch=["res"])
        print(ret)
        l_end = time.time()
        latency_list.append(l_end * 1000 - l_start * 1000)
        total_number = total_number + 1
    end = time.time()
    return [[end - start], latency_list, [total_number]]


def multithread_rpc(thraed, batch_size):
    multi_thread_runner = MultiThreadRunner()
    start = time.time()
    result = multi_thread_runner.run(run_rpc, thread, batch_size)
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
    print("AVG QPS: {} samples/s".format(batch_size * total_number /
                                         total_cost))
    show_latency(result[1])


if __name__ == "__main__":
    if sys.argv[1] == "yaml":
        mode = sys.argv[2]  # brpc/  local predictor
        thread = int(sys.argv[3])
        device = sys.argv[4]
        gen_yml(device)
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
