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
import sys
import cv2
import json
import os
import numpy as np


def predict(image_path, server):
    image = open(image_path).read()
    image = base64.b64encode(image)

    req = {}
    req["image"] = image
    req["fetch"] = ["score"]

    req = json.dumps(req)
    url = server
    headers = {"Content-Type": "application/json"}
    r = requests.post(url, data=req, headers=headers)
    if r.status_code == requests.codes.ok:
        score = r.json()["score"]
        score = np.array(score)
        print("picture {} max score : {} class {}".format(
            image_path, np.max(score), np.argmax(score)))
    else:
        print("predict {} error".format(image_path))


if __name__ == "__main__":
    server = "http://127.0.0.1:9393/image/prediction"
    image_path = "./data/n01440764_10026.JPEG"
    predict(image_path, server)
