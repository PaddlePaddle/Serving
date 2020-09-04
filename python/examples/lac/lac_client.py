# encoding=utf-8
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
# pylint: disable=doc-string-missing

from paddle_serving_client import Client
from paddle_serving_app.reader import LACReader
import sys
import os
import io
import numpy as np

client = Client()
client.load_client_config(sys.argv[1])
client.connect(["127.0.0.1:9292"])

reader = LACReader()
for line in sys.stdin:
    if len(line) <= 0:
        continue
    feed_data = reader.process(line)
    if len(feed_data) <= 0:
        continue
    print(feed_data)
    #fetch_map = client.predict(feed={"words": np.array(feed_data).reshape(len(feed_data), 1), "words.lod": [0, len(feed_data)]}, fetch=["crf_decode"], batch=True)
    fetch_map = client.predict(
        feed={
            "words": np.array(feed_data + feed_data).reshape(
                len(feed_data) * 2, 1),
            "words.lod": [0, len(feed_data), 2 * len(feed_data)]
        },
        fetch=["crf_decode"],
        batch=True)
    print(fetch_map)
    begin = fetch_map['crf_decode.lod'][0]
    end = fetch_map['crf_decode.lod'][1]
    segs = reader.parse_result(line, fetch_map["crf_decode"][begin:end])
    print("word_seg: " + "|".join(str(words) for words in segs))
