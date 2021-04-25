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
from paddle_serving_app.reader import Sequential, Resize, SegPostprocess
try:
    from paddle_serving_server_gpu.web_service import WebService, Op
except ImportError:
    from paddle_serving_server.web_service import WebService, Op
import logging
import numpy as np
import base64, cv2


class SegOp(Op):
    def init_op(self):
        self.seq = Sequential([Resize((512, 512), interpolation=cv2.INTER_LINEAR)])
        self.seg_postprocess=SegPostprocess(2)

    def preprocess(self, input_dicts, data_id, log_id):
        (_, input_dict), = input_dicts.items()
        batch_size = len(input_dict.keys())
        imgs = []
        for key in input_dict.keys():
            data = base64.b64decode(input_dict[key].encode('utf8'))
            data = np.fromstring(data, np.uint8)
            im = cv2.imdecode(data, cv2.IMREAD_COLOR)
            img = self.seq(im)
            imgs.append(img[np.newaxis, :].copy())
        input_imgs = np.concatenate(imgs, axis=0)
        print(input_imgs.shape)
        return {"seg": input_imgs}, False, None, ""

    def postprocess(self, input_dicts, fetch_dict, log_id):
        print(fetch_dict)
        return fetch_dict["output"].tolist(), None, ""


class ImageService(WebService):
    def get_pipeline_response(self, read_op):
        seg_op = SegOp(name="seg", input_ops=[read_op])
        return seg_op


uci_service = ImageService(name="seg")
uci_service.prepare_pipeline_config("config2.yml")
uci_service.run_service()
