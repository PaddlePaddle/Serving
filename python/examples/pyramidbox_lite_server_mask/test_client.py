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
# -*- coding: utf-8 -*-

from paddle_serving_client import Client
import sys
from process import DetectFace

DetectFace = DetectFace("output")
client = Client()
# load client prototxt
client.load_client_config(sys.argv[1])
client.connect(['127.0.0.1:9494'])
image, _, _ = DetectFace.preprocess(sys.argv[2])
fetch_map = client.predict(
    feed={"image": image}, fetch=["save_infer_model/scale_0"])
fetch_map["image"] = sys.argv[2]
fetch_map["im_shape"] = image.shape
DetectFace.postprocess(fetch_map)
