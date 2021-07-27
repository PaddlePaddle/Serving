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
