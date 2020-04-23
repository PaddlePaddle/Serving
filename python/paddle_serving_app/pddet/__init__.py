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

import os
import time
import argparse
from .image_tool import Resize, Detection


def ArgParse():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--config_path", type=str, default=None, help="preprocess config path.")
    parser.add_argument(
        "--infer_img", type=str, default=None, help="Image path")
    parser.add_argument(
        "--visualize",
        action='store_true',
        default=False,
        help="Whether to visualize detection output")
    parser.add_argument(
        "--output_dir",
        type=str,
        default="output",
        help="Directory for storing the output visualization files.")
    parser.add_argument(
        "--serving_client_conf",
        type=str,
        default='pddet_client_conf/serving_client_conf.prototxt',
        help='Paddle Serving Client configuration')
    parser.add_argument(
        "--dump_result",
        action='store_true',
        default=False,
        help="Whether to dump result")
    global FLAGS
    FLAGS = parser.parse_args()
    return FLAGS
