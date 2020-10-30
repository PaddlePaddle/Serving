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
from paddle_serving_app.reader import *
import sys
import numpy as np
from paddle_serving_app.reader import BlazeFacePostprocess

preprocess = Sequential([
    File2Image(),
    Normalize([104, 117, 123], [127.502231, 127.502231, 127.502231], False)
])

postprocess = BlazeFacePostprocess("label_list.txt", "output")
client = Client()

client.load_client_config(sys.argv[1])
client.connect(['127.0.0.1:9494'])

im_0 = preprocess(sys.argv[2])
tmp = Transpose((2, 0, 1))
im = tmp(im_0)
fetch_map = client.predict(
    feed={"image": im}, fetch=["detection_output_0.tmp_0"])
fetch_map["image"] = sys.argv[2]
fetch_map["im_shape"] = im_0.shape
postprocess(fetch_map)
