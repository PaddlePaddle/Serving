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
from paddle_serving_app.reader import OCRReader
import cv2

client = Client()
client.load_client_config("ocr_rec_client/serving_client_conf.prototxt")
client.connect(["127.0.0.1:9292"])

image_file_list = ["./test_rec.jpg"]
img = cv2.imread(image_file_list[0])
ocr_reader = OCRReader()
feed = {"image": ocr_reader.preprocess([img])}
fetch = ["ctc_greedy_decoder_0.tmp_0", "softmax_0.tmp_0"]
fetch_map = client.predict(feed=feed, fetch=fetch)
rec_res = ocr_reader.postprocess(fetch_map)
print(image_file_list[0])
print(rec_res[0][0])
