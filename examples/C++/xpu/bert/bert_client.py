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

import sys
from paddle_serving_client import Client
from paddle_serving_client.utils import benchmark_args
from chinese_bert_reader import ChineseBertReader
import numpy as np
args = benchmark_args()

reader = ChineseBertReader({"max_seq_len": 128})
fetch = ["save_infer_model/scale_0.tmp_1"]
endpoint_list = ['127.0.0.1:7703']
client = Client()
client.load_client_config(args.model)
client.connect(endpoint_list)

for line in sys.stdin:
    feed_dict = reader.process(line)
    for key in feed_dict.keys():
        feed_dict[key] = np.array(feed_dict[key]).reshape((1, 128))
    #print(feed_dict)
    result = client.predict(feed=feed_dict, fetch=fetch, batch=True)
print(result)
