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


def cv2_to_base64(image):
    return base64.b64encode(image).decode('utf8')


test_img_dir = "imgs/"
for img_file in os.listdir(test_img_dir):
    with open(os.path.join(test_img_dir, img_file), 'rb') as file:
        image_data = file.read()
    # print file name
    print('{}{}{}'.format('*' * 10, img_file, '*' * 10))
    image = cv2_to_base64(image_data)

    result = client.predict(feed_dict={"image": image}, fetch=["res"])
    print("erro_no:{}, err_msg:{}".format(result.err_no, result.err_msg))
    # check success
    if result.err_no == 0:
        ocr_result = result.value[0]
        try:
            for item in eval(ocr_result):
                # return transcription and points
                print("{}, {}".format(item[0], item[1]))
        except Exception as e:
            print("No results")
            continue

    else:
        print(
            "For details about error message, see PipelineServingLogs/pipeline.log.wf"
        )
