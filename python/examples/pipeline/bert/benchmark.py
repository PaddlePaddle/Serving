import sys
import os
import yaml
import requests
import time
import json
try:
    from paddle_serving_server_gpu.pipeline import PipelineClient
except ImportError:
    from paddle_serving_server.pipeline import PipelineClient
import numpy as np
from paddle_serving_client.utils import MultiThreadRunner
from paddle_serving_client.utils import benchmark_args, show_latency

def gen_yml(device):
    fin = open("config.yml", "r")
    config = yaml.load(fin)
    fin.close()
    config["dag"]["tracer"] = {"interval_s": 20}
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
        end = time.time()
    return [[end - start]]

def multithread_http(thread, batch_size):
    multi_thread_runner = MultiThreadRunner()
    result = multi_thread_runner.run(run_http , thread, batch_size)

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
        end = time.time()
    return [[end - start]]


def multithread_rpc(thraed, batch_size):
    multi_thread_runner = MultiThreadRunner()
    result = multi_thread_runner.run(run_rpc , thread, batch_size)

if __name__ == "__main__":
    if sys.argv[1] == "yaml":
        mode = sys.argv[2] # brpc/  local predictor
        thread = int(sys.argv[3])
        device = sys.argv[4]
        gen_yml(device)
    elif sys.argv[1] == "run":
        mode = sys.argv[2] # http/ rpc
        thread = int(sys.argv[3])
        batch_size = int(sys.argv[4])
        if mode == "http":
            multithread_http(thread, batch_size)
        elif mode == "rpc":
            multithread_rpc(thread, batch_size)

    
