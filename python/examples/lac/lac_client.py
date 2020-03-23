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
from lac_reader import LACReader
import sys
import os
import io

client = Client()
client.load_client_config(sys.argv[1])
client.add_variant("var1", ["127.0.0.1:9280"], 50)
client.connect()

reader = LACReader(sys.argv[2])
for line in sys.stdin:
    if len(line) <= 0:
        continue
    feed_data = reader.process(line)
    if len(feed_data) <= 0:
        continue
    fetch_map = client.predict(feed={"words": feed_data}, fetch=["crf_decode"])
    print(fetch_map)
