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


def predict(image_path, server):
    image = base64.b64encode(open(image_path).read())
    req = json.dumps({"image": image, "fetch": ["score"]})
    r = requests.post(
        server, data=req, headers={"Content-Type": "application/json"})


if __name__ == "__main__":
    server = "http://127.0.0.1:9393/image/prediction"
    image_path = "./data/n01440764_10026.JPEG"
    start = time.time()
    for i in range(1000):
        predict(image_path, server)
    end = time.time()
    print(end - start)
