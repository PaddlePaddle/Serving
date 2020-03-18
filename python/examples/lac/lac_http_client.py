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
#coding=utf-8
import requests
import json
import time

if __name__ == "__main__":
    server = "http://127.0.0.1:9280/lac/prediction"
    fin = open("jieba_test.txt", "r")
    start = time.time()
    for line in fin:
        req_data = {"words": line.strip(), "fetch": ["crf_decode"]}
        r = requests.post(server, json=req_data)
    end = time.time()
    print(end - start)
