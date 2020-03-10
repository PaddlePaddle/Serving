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
from image_reader import ImageReader
from paddle_serving_client import Client

client = Client()
client.load_client_config(sys.argv[1])
client.connect(["127.0.0.1:9393"])
reader = ImageReader()
with open("./data/n01440764_10026.JPEG") as f:
    img = f.read()

img = reader.process_image(img).reshape(-1)
fetch_map = client.predict(feed={"image": img}, fetch=["score"])

print(fetch_map["score"])
