#   Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
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

import cv2
import numpy as np


def transpose(img, transpose_target):
    img = img.transpose(transpose_target)
    return img


def normalize(img, mean, std, channel_first):
    # need to optimize here
    if channel_first:
        img_mean = np.array(mean).reshape((3, 1, 1))
        img_std = np.array(std).reshape((3, 1, 1))
    else:
        img_mean = np.array(mean).reshape((1, 1, 3))
        img_std = np.array(std).reshape((1, 1, 3))
    img = np.array(img).astype("float32")
    img -= img_mean
    img /= img_std
    return img


def crop(img, target_size, center):
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


def resize(img, target_size, max_size=2147483647, interpolation=None):
    if isinstance(target_size, tuple):
        resized_width = min(target_size[0], max_size)
        resized_height = min(target_size[1], max_size)
    else:
        im_max_size = max(img.shape[0], img.shape[1])
        percent = float(target_size) / min(img.shape[0], img.shape[1])
        if np.round(percent * im_max_size) > max_size:
            percent = float(max_size) / float(im_max_size)
        resized_width = int(round(img.shape[1] * percent))
        resized_height = int(round(img.shape[0] * percent))
    if interpolation:
        resized = cv2.resize(
            img, (resized_width, resized_height), interpolation=interpolation)
    else:
        resized = cv2.resize(img, (resized_width, resized_height))
    return resized
