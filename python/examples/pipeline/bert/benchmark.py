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
import yaml
import requests
import time
import json

from paddle_serving_server.pipeline import PipelineClient
import numpy as np
from paddle_serving_client.utils import MultiThreadRunner
from paddle_serving_client.utils import benchmark_args, show_latency
'''
2021-03-16 10:26:01,832 ==================== TRACER ======================
2021-03-16 10:26:01,838 Op(bert):
2021-03-16 10:26:01,838 	in[5.7833 ms]
2021-03-16 10:26:01,838 	prep[8.2001 ms]
2021-03-16 10:26:01,838 	midp[198.79853333333332 ms]
2021-03-16 10:26:01,839 	postp[0.8411 ms]
2021-03-16 10:26:01,839 	out[0.9440666666666667 ms]
2021-03-16 10:26:01,839 	idle[0.03135320683677345]
2021-03-16 10:26:01,839 DAGExecutor:
2021-03-16 10:26:01,839 	Query count[30]
2021-03-16 10:26:01,839 	QPS[3.0 q/s]
2021-03-16 10:26:01,839 	Succ[1.0]
2021-03-16 10:26:01,839 	Error req[]
2021-03-16 10:26:01,839 	Latency:
2021-03-16 10:26:01,839 		ave[237.85519999999997 ms]
2021-03-16 10:26:01,839 		.50[179.937 ms]
2021-03-16 10:26:01,839 		.60[179.994 ms]
2021-03-16 10:26:01,839 		.70[180.515 ms]
2021-03-16 10:26:01,840 		.80[180.735 ms]
2021-03-16 10:26:01,840 		.90[182.275 ms]
2021-03-16 10:26:01,840 		.95[182.789 ms]
2021-03-16 10:26:01,840 		.99[1921.33 ms]
2021-03-16 10:26:01,840 Channel (server worker num[1]):
2021-03-16 10:26:01,840 	chl0(In: ['@DAGExecutor'], Out: ['bert']) size[0/0]
2021-03-16 10:26:01,841 	chl1(In: ['bert'], Out: ['@DAGExecutor']) size[0/0]
'''


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
        config["op"]["bert"]["local_service_conf"]["device_type"] = 1
        config["op"]["bert"]["local_service_conf"]["devices"] = "2"
    with open("config2.yml", "w") as fout:
        yaml.dump(config, fout, default_flow_style=False)


def run_http(idx, batch_size):
    print("start thread ({})".format(idx))
    url = "http://127.0.0.1:18082/bert/prediction"
    start = time.time()
    with open("data-c.txt", 'r') as fin:
        start = time.time()
        lines = fin.readlines()
        start_idx = 0
        while start_idx < len(lines):
            end_idx = min(len(lines), start_idx + batch_size)
            feed = {}
            for i in range(start_idx, end_idx):
                feed[str(i - start_idx)] = lines[i]
            keys = list(feed.keys())
            values = [feed[x] for x in keys]
            data = {"key": keys, "value": values}
            r = requests.post(url=url, data=json.dumps(data))
            start_idx += batch_size
            if start_idx > 2000:
                break
        end = time.time()
    return [[end - start]]


def multithread_http(thread, batch_size):
    multi_thread_runner = MultiThreadRunner()
    result = multi_thread_runner.run(run_http, thread, batch_size)


def run_rpc(thread, batch_size):
    client = PipelineClient()
    client.connect(['127.0.0.1:9998'])
    with open("data-c.txt", 'r') as fin:
        start = time.time()
        lines = fin.readlines()
        start_idx = 0
        while start_idx < len(lines):
            end_idx = min(len(lines), start_idx + batch_size)
            feed = {}
            for i in range(start_idx, end_idx):
                feed[str(i - start_idx)] = lines[i]
            ret = client.predict(feed_dict=feed, fetch=["res"])
            start_idx += batch_size
            if start_idx > 1000:
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
