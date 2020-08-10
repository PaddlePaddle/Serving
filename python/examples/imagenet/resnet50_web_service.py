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
from paddle_serving_client import Client
from paddle_serving_app.reader import Sequential, URL2Image, Resize, CenterCrop, RGB2BGR, Transpose, Div, Normalize

if len(sys.argv) != 4:
    print("python resnet50_web_service.py model device port")
    sys.exit(-1)

device = sys.argv[2]

if device == "cpu":
    from paddle_serving_server.web_service import WebService
else:
    from paddle_serving_server_gpu.web_service import WebService


class ImageService(WebService):
    def init_imagenet_setting(self):
        self.seq = Sequential([
            URL2Image(), Resize(256), CenterCrop(224), RGB2BGR(), Transpose(
                (2, 0, 1)), Div(255), Normalize([0.485, 0.456, 0.406],
                                                [0.229, 0.224, 0.225], True)
        ])
        self.label_dict = {}
        label_idx = 0
        with open("imagenet.label") as fin:
            for line in fin:
                self.label_dict[label_idx] = line.strip()
                label_idx += 1

    def preprocess(self, feed=[], fetch=[]):
        feed_batch = []
        for ins in feed:
            if "image" not in ins:
                raise ("feed data error!")
            img = self.seq(ins["image"])
            feed_batch.append({"image": img})
        return feed_batch, fetch

    def postprocess(self, feed=[], fetch=[], fetch_map={}):
        score_list = fetch_map["score"]
        result = {"label": [], "prob": []}
        for score in score_list:
            score = score.tolist()
            max_score = max(score)
            result["label"].append(self.label_dict[score.index(max_score)]
                                   .strip().replace(",", ""))
            result["prob"].append(max_score)
        return result


image_service = ImageService(name="image")
image_service.load_model_config(sys.argv[1])
image_service.init_imagenet_setting()
if device == "gpu":
    image_service.set_gpus("0")
image_service.prepare_server(
    workdir="workdir", port=int(sys.argv[3]), device=device)
image_service.run_rpc_service()
image_service.run_web_service()
