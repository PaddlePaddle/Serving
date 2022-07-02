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
import sys
import numpy as np
import base64
import os
import cv2
from paddle_serving_app.reader import Sequential, URL2Image, ResizeByFactor
from paddle_serving_app.reader import Div, Normalize, Transpose
from paddle_serving_app.reader import OCRReader

client = Client()
# TODO:load_client need to load more than one client model.
# this need to figure out some details.
client.load_client_config(sys.argv[1:])
client.connect(["127.0.0.1:9293"])

import paddle
test_img_dir = "imgs/"


def cv2_to_base64(image):
    return base64.b64encode(image)  #data.tostring()).decode('utf8')


for img_file in os.listdir(test_img_dir):
    with open(os.path.join(test_img_dir, img_file), 'rb') as file:
        image_data = file.read()
    image = cv2_to_base64(image_data)
    fetch_map = client.predict(
        feed={"x": image},
        fetch=["save_infer_model/scale_0.tmp_1"],
        batch=True)
    result = {}
    rec_res = OCRReader().postprocess_ocrv2(fetch_map, with_score=False)
    res_lst = []
    for res in rec_res:
        res_lst.append(res[0])
    result["res"] = res_lst

    print(result)
