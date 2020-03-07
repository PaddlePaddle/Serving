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

from __future__ import unicode_literals, absolute_import
import os
import sys
import time
from paddle_serving_client import Client
from paddle_serving_client.utils import MultiThreadRunner
from paddle_serving_client.utils import benchmark_args
from batching import pad_batch_data
import tokenization
import requests
import json
from bert_reader import BertReader

args = benchmark_args()

def single_func(idx, resource):
    fin = open("data-c.txt")
    if args.request == "rpc":
        reader = BertReader(vocab_file="vocab.txt", max_seq_len=20)
        config_file = './serving_client_conf/serving_client_conf.prototxt'
        fetch = ["pooled_output"]
        client = Client()
        client.load_client_config(args.model)
        client.connect([resource["endpoint"][idx % 4]])
        
        start = time.time()
        for line in fin:
            feed_dict = reader.process(line)
            result = client.predict(feed=feed_dict,
                                    fetch=fetch)
        end = time.time()
    elif args.request == "http":
        start = time.time()
        header = {"Content-Type":"application/json"}
        for line in fin:
            #dict_data = {"words": "this is for output ", "fetch": ["pooled_output"]}
            dict_data = {"words": line, "fetch": ["pooled_output"]}
            r = requests.post('http://{}/bert/prediction'.format(resource["endpoint"][0]),
                              data=json.dumps(dict_data), headers=header)
        end = time.time()
    return [[end - start]]

if __name__ == '__main__':
    multi_thread_runner = MultiThreadRunner()
    endpoint_list = ["127.0.0.1:9494", "127.0.0.1:9495", "127.0.0.1:9496", "127.0.0.1:9497"]
    #endpoint_list = endpoint_list + endpoint_list + endpoint_list
    #result = multi_thread_runner.run(single_func, args.thread, {"endpoint":endpoint_list})
    result = single_func(0, {"endpoint":endpoint_list})
    print(result)

