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
import numpy as np

image = open("./to_longteng/n01440764/n01440764_12362.JPEG").read()
image = base64.b64encode(image)

req = {}
req["image"] = image
req["fetch"] = ["score"]

req = json.dumps(req)
url = "http://127.0.0.1:9291/image/prediction"
headers = {"Content-Type": "application/json"}
r = requests.post(url, data=req, headers=headers)
score = r.json()["score"]
score = np.array(score)
print("max score : {} class {}".format(np.max(score), np.argmax(score)))
