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

preprocess = Sequential([
    File2Image(), BGR2RGB(), Div(255.0),
    Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225], False),
    Resize(640, 640), Transpose((2, 0, 1))
])

postprocess = RCNNPostprocess("label_list.txt", "output")
client = Client()

client.load_client_config(sys.argv[1])
client.connect(['127.0.0.1:9494'])

im = preprocess(sys.argv[3])
fetch_map = client.predict(
    feed={
        "image": im,
        "im_info": np.array(list(im.shape[1:]) + [1.0]),
        "im_shape": np.array(list(im.shape[1:]) + [1.0])
    },
    fetch=["multiclass_nms"])
fetch_map["image"] = sys.argv[3]
postprocess(fetch_map)
