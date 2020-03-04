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


class ImageService(WebService):
    def set_param(self):
        self.image_mean = [0.485, 0.456, 0.406]
        self.image_std = [0.229, 0.224, 0.225]
        self.image_shape = [3, 224, 224]
        self.resize_short_size = 256
        self.interpolation = None

    def resize_short(self, img, target_size, interpolation=None):
        """resize image

        Args:
            img: image data
            target_size: resize short target size
            interpolation: interpolation mode

        Returns:
            resized image data
        """
        percent = float(target_size) / min(img.shape[0], img.shape[1])
        resized_width = int(round(img.shape[1] * percent))
        resized_height = int(round(img.shape[0] * percent))
        if interpolation:
            resized = cv2.resize(
                img, (resized_width, resized_height),
                interpolation=interpolation)
        else:
            resized = cv2.resize(img, (resized_width, resized_height))
        return resized

    def crop_image(self, img, target_size, center):
        """crop image

        Args:
            img: images data
            target_size: crop target size
            center: crop mode

        Returns:
            img: cropped image data
        """
        height, width = img.shape[:2]
        size = target_size
        if center == True:
            w_start = (width - size) // 2
            h_start = (height - size) // 2
        else:
            w_start = np.random.randint(0, width - size + 1)
            h_start = np.random.randint(0, height - size + 1)
        w_end = w_start + size
        h_end = h_start + size
        img = img[h_start:h_end, w_start:w_end, :]
        return img

    def process_image(self, sample):
        """ process_image """
        mean = self.image_mean
        std = self.image_std
        crop_size = self.image_shape[1]

        data = np.fromstring(sample, np.uint8)
        img = cv2.imdecode(data, cv2.IMREAD_COLOR)

        if img is None:
            print("img is None, pass it.")
            return None

        if crop_size > 0:
            target_size = self.resize_short_size
            img = self.resize_short(
                img, target_size, interpolation=self.interpolation)
            img = self.crop_image(img, target_size=crop_size, center=True)

        img = img[:, :, ::-1]

        img = img.astype('float32').transpose((2, 0, 1)) / 255
        img_mean = np.array(mean).reshape((3, 1, 1))
        img_std = np.array(std).reshape((3, 1, 1))
        img -= img_mean
        img /= img_std
        return img

    def preprocess(self, feed={}, fetch=[]):
        self.set_param()
        if "image" not in feed:
            raise ("feed data error!")
        sample = base64.b64decode(feed["image"])
        img = self.process_image(sample)
        res_feed = {}
        res_feed["image"] = img.reshape(-1)
        return res_feed, fetch


image_service = ImageService(name="image")
image_service.load_model_config(sys.argv[1])
image_service.prepare_server(workdir=sys.argv[2], port=9393, device="cpu")
image_service.run_server()
