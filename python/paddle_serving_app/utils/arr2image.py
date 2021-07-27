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
# pylint: disable=doc-string-missing

import cv2
import yaml


class Arr2Image(object):
    """
    from numpy array image(jpeg) to cv::Mat image
    """

    def __init__(self):
        pass

    def __call__(self, img_arr):
        img = cv2.imdecode(img_arr, cv2.IMREAD_COLOR)
        return img

    def __repr__(self):
        return self.__class__.__name__ + "()"
