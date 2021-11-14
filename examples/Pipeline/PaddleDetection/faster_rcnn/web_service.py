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
from paddle_serving_server.web_service import WebService, Op
import logging
import numpy as np
import sys
import cv2
from paddle_serving_app.reader import *
import base64


class FasterRCNNOp(Op):
    def init_op(self):
        self.img_preprocess = Sequential([
            BGR2RGB(), Div(255.0),
            Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225], False),
            Resize(640, 640), Transpose((2, 0, 1))
        ])
        self.img_postprocess = RCNNPostprocess("label_list.txt", "output")

    def preprocess(self, input_dicts, data_id, log_id):
        (_, input_dict), = input_dicts.items()
        imgs = []
        #print("keys", input_dict.keys())
        for key in input_dict.keys():
            data = base64.b64decode(input_dict[key].encode('utf8'))
            data = np.fromstring(data, np.uint8)
            im = cv2.imdecode(data, cv2.IMREAD_COLOR)
            im = self.img_preprocess(im)
            imgs.append({
                "image": im[np.newaxis, :],
                "im_shape":
                np.array(list(im.shape[1:])).reshape(-1)[np.newaxis, :],
                "scale_factor": np.array([1.0, 1.0]).reshape(-1)[np.newaxis, :],
            })
        feed_dict = {
            "image": np.concatenate(
                [x["image"] for x in imgs], axis=0),
            "im_shape": np.concatenate(
                [x["im_shape"] for x in imgs], axis=0),
            "scale_factor": np.concatenate(
                [x["scale_factor"] for x in imgs], axis=0)
        }
        #for key in feed_dict.keys():
        #    print(key, feed_dict[key].shape)
        return feed_dict, False, None, ""

    def postprocess(self, input_dicts, fetch_dict, data_id, log_id):
        #print(fetch_dict)
        res_dict = {
            "bbox_result":
            str(self.img_postprocess(
                fetch_dict, visualize=False))
        }
        return res_dict, None, ""


class FasterRCNNService(WebService):
    def get_pipeline_response(self, read_op):
        faster_rcnn_op = FasterRCNNOp(name="faster_rcnn", input_ops=[read_op])
        return faster_rcnn_op


fasterrcnn_service = FasterRCNNService(name="faster_rcnn")
fasterrcnn_service.prepare_pipeline_config("config.yml")
fasterrcnn_service.run_service()
