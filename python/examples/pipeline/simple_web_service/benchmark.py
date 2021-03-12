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

def gen_yml():
    fin = open("config.yml", "r")
    config = yaml.load(fin)
    fin.close()
    config["dag"]["tracer"] = {"interval_s": 5}
    with open("config2.yml", "w") as fout: 
        yaml.dump(config, fout, default_flow_style=False)

def run_http(idx, batch_size):
    print("start thread ({})".format(idx))
    url = "http://127.0.0.1:18082/uci/prediction"    
    start = time.time()
    data = {"key": ["x"], "value": ["0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332"]}
    for i in range(1000):
        r = requests.post(url=url, data=json.dumps(data))
    print(r.json())
    end = time.time()
    return [[end - start]]

def multithread_http(thread, batch_size):
    multi_thread_runner = MultiThreadRunner()
    result = multi_thread_runner.run(
        run_http , thread, batch_size)


def multithread_rpc(thraed, batch_size):
    multi_thread_runner = MultiThreadRunner()


def run_rpc(thread, batch_size):
    client = PipelineClient()
    client.connect(['127.0.0.1:9998'])
    data = {"key": "x", "value": "0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332"}
    ret = client.predict(feed_dict={data["key"]: data["value"]}, fetch=["res"])
    print(ret)

if __name__ == "__main__":
    if sys.argv[1] == "yaml":
        mode = sys.argv[2] # brpc/  local predictor
        thread = int(sys.argv[3])
        gen_yml()
    elif sys.argv[1] == "run":
        mode = sys.argv[2] # http/ rpc
        thread = int(sys.argv[3])
        batch_size = int(sys.argv[4])
        if mode == "http":
            multithread_http(thread, batch_size)
        elif mode == "rpc":
            run_rpc(thread, batch_size)

    
