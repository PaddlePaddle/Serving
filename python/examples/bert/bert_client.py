# coding:utf-8
# pylint: disable=doc-string-missing
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

import os
import sys
import numpy as np
import paddlehub as hub
import ujson
import random
import time
from paddlehub.common.logger import logger
import socket
from paddle_serving_client import Client
from paddle_serving_client.utils import benchmark_args
from bert_reader import BertReader

args = benchmark_args()

fin = open("data-c.txt")
reader = BertReader(vocab_file="vocab.txt", max_seq_len=128)
fetch = ["pooled_output"]
endpoint_list = ["127.0.0.1:9494"]
client = Client()
client.load_client_config(args.model)
client.connect(endpoint_list)

for line in fin:
    feed_dict = reader.process(line)
    result = client.predict(feed=feed_dict, fetch=fetch)
