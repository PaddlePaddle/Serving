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
try:
    from paddle_serving_server.pipeline import PipelineClient
except ImportError:
    from paddle_serving_server.pipeline import PipelineClient
import numpy as np
import requests
import json
import cv2
import base64
import os

client = PipelineClient()
client.connect(['127.0.0.1:18090'])

video_url = "https://paddle-serving.bj.bcebos.com/model/PaddleVideo/example.avi"
for i in range(1):
    ret = client.predict(feed_dict={"video_url": video_url}, fetch=["res"])
    print(ret)
