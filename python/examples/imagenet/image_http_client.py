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

import requests
import base64
import json
import time
import os


def predict(image_path, server):
    image = base64.b64encode(open(image_path).read())
    req = json.dumps({"image": image, "fetch": ["score"]})
    r = requests.post(
        server, data=req, headers={"Content-Type": "application/json"})
    print(r.json()["score"][0])
    return r


def batch_predict(image_path, server):
    image = base64.b64encode(open(image_path).read())
    req = json.dumps({"image": [image, image], "fetch": ["score"]})
    r = requests.post(
        server, data=req, headers={"Content-Type": "application/json"})
    print(r.json()["result"][1]["score"][0])
    return r


if __name__ == "__main__":
    server = "http://127.0.0.1:9393/image/prediction"
    #image_path = "./data/n01440764_10026.JPEG"
    image_list = os.listdir("./image_data/n01440764/")
    start = time.time()
    for img in image_list:
        image_file = "./image_data/n01440764/" + img
        res = predict(image_file, server)
    end = time.time()
    print(end - start)
