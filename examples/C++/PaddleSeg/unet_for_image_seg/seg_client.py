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

from paddle_serving_client import Client
from paddle_serving_app.reader import Sequential, File2Image, Resize, Transpose, BGR2RGB, SegPostprocess
import sys
import cv2

client = Client()
client.load_client_config("unet_client/serving_client_conf.prototxt")
client.connect(["127.0.0.1:9494"])

preprocess = Sequential(
    [File2Image(), Resize(
        (512, 512), interpolation=cv2.INTER_LINEAR)])

postprocess = SegPostprocess(2)

filename = "N0060.jpg"
im = preprocess(filename)
fetch_map = client.predict(feed={"image": im}, fetch=["output"])
fetch_map["filename"] = filename
postprocess(fetch_map)
