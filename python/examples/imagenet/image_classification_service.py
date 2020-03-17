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

from paddle_serving_server.web_service import WebService
import sys
import cv2
import base64
import numpy as np
from image_reader import ImageReader


class ImageService(WebService):
    def preprocess(self, feed={}, fetch=[]):
        reader = ImageReader()
        if "image" not in feed:
            raise ("feed data error!")
        sample = base64.b64decode(feed["image"])
        img = reader.process_image(sample)
        res_feed = {}
        res_feed["image"] = img.reshape(-1)
        return res_feed, fetch


image_service = ImageService(name="image")
image_service.load_model_config(sys.argv[1])
image_service.prepare_server(
    workdir=sys.argv[2], port=int(sys.argv[3]), device="cpu")
image_service.run_server()
