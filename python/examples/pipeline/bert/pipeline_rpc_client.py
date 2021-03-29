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


client = PipelineClient()
client.connect(['127.0.0.1:9998'])
batch_size = 101
with open("data-c.txt", 'r') as fin:
     lines = fin.readlines()
     start_idx = 0
     while start_idx < len(lines):
         end_idx = min(len(lines), start_idx + batch_size)
         feed = {}
         for i in range(start_idx, end_idx):
             feed[str(i - start_idx)] = lines[i]
         ret = client.predict(feed_dict=feed, fetch=["res"])
         print(ret)
         start_idx += batch_size
