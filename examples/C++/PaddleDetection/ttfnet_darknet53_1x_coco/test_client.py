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
import numpy as np
from paddle_serving_client import Client
from paddle_serving_app.reader import *
import cv2

preprocess = DetectionSequential([
    DetectionFile2Image(), DetectionResize(
        (512, 512), False, interpolation=cv2.INTER_LINEAR),
    DetectionNormalize([123.675, 116.28, 103.53], [58.395, 57.12, 57.375],
                       False), DetectionTranspose((2, 0, 1))
])

postprocess = RCNNPostprocess("label_list.txt", "output")
client = Client()

client.load_client_config("serving_client/serving_client_conf.prototxt")
client.connect(['127.0.0.1:9494'])

im, im_info = preprocess(sys.argv[1])

fetch_map = client.predict(
    feed={
        "image": im,
        "im_shape": np.array(list(im.shape[1:])).reshape(-1),
        "scale_factor": im_info['scale_factor'],
    },
    fetch=["save_infer_model/scale_0.tmp_1"],
    batch=False)
print(fetch_map)
