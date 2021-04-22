import sys
import os
import yaml
import requests
import time
import json
import ast
from paddle_serving_client.utils import MultiThreadRunner
from paddle_serving_client.utils import benchmark_args, show_latency

def parse_benchmark(filein, fileout):
    with open(filein, "r") as fin:
        res = yaml.load(fin)
        del_list = []
        for key in res["DAG"].keys():
            if "call" in key:
                del_list.append(key)
        for key in del_list:
            del res["DAG"][key]
    with open(fileout, "w") as fout:
        yaml.dump(res, fout, default_flow_style=False)

def run_http(idx, batch_size):
    """
    {"feed":[{"words": "hello"}], "fetch":["pooled_output"]}
    """
    print("start thread ({})".format(idx))
    url = "http://127.0.0.1:9696/bert/prediction"    
    start = time.time()
    with open("data-c.txt", 'r') as fin:
        start = time.time()
        lines = fin.readlines()
        start_idx = 0
        while start_idx < len(lines):
            end_idx = min(len(lines), start_idx + batch_size)
            feed = {}
            feed_lst = [{"words": lines[i]} for i in range(start_idx, end_idx)]
            data = {"feed": feed_lst, "fetch": ["pooled_output"]}
            r = requests.post(url=url, data=json.dumps(data), headers={"Content-Type": "application/json"})
            start_idx += batch_size
            end = time.time()
            if end - start > 15:
                break            
        end = time.time()
    return [[end - start]]

def multithread_http(thread, batch_size):
    multi_thread_runner = MultiThreadRunner()
    result = multi_thread_runner.run(run_http , thread, batch_size)

if __name__ == "__main__":
   if sys.argv[1] == "run":
       thread = int(sys.argv[2])
       batch_size = int(sys.argv[3])
       multithread_http(thread, batch_size)
   if sys.argv[1] == "dump":
       filein = sys.argv[2]
       fileout = sys.argv[3]
       parse_benchmark(filein, fileout)
