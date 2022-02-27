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
import cv2
import base64
from paddle_serving_app.reader import VideoDecoder, Sampler, Scale, TenCrop, Image2Array, Normalization
import urllib.request
_LOGGER = logging.getLogger()


class ppTSNOp(Op):
    def init_op(self,
                num_seg=25,
                seg_len=1,
                short_size=256,
                target_size=224,
                top_k=1):
        self.top_k = top_k
        img_mean = [0.485, 0.456, 0.406]
        img_std = [0.229, 0.224, 0.225]
        self.ops = [
            VideoDecoder(),
            Sampler(num_seg, seg_len, valid_mode=True, select_left=True),
            Scale(short_size, fixed_ratio=True, do_round=True, backend='cv2'),
            TenCrop(target_size),
            Image2Array(),
            Normalization(img_mean, img_std)                                
        ]
        self.label_dict = {}
        with open("Kinetics-400_label_list.txt") as fin:
            for line in fin:
                label_list = line.strip().split(" ")
                index = int(label_list[0])
                label = label_list[1]
                self.label_dict[index] = label

    def preprocess(self, input_dicts, data_id, log_id):
        (_, input_dict), = input_dicts.items()
        self.input_file = []
        for key in input_dict.keys():
            try:
                filename = urllib.request.urlretrieve(input_dict[key], key)
                self.input_file.append(filename[0])
                print("download video success")
            except:
                print("download video failed")
        batched_inputs = []
        for filename in self.input_file:
            results = {'filename': filename}
            for op in self.ops:
                results = op(results)
            res = np.expand_dims(results['imgs'], axis=0).copy()
            batched_inputs.append(res)
        batched_inputs = [
            np.concatenate([item[i] for item in batched_inputs])
            for i in range(len(batched_inputs[0]))
        ]
        return {"data_batch_0": batched_inputs[0]}, False, None, ""

    def postprocess(self, input_dicts, fetch_dict, data_id, log_id):
        output = fetch_dict["linear_1.tmp_1"]
        if not isinstance(self.input_file, list):
            self.input_file = [
                self.input_file,
            ]
        N = len(self.input_file)
        if output.shape[0] != N:
            output = output.reshape([N] + [output.shape[0] // N] +
                                    list(output.shape[1:]))  # [N, T, C]
            output = output.mean(axis=1)  # [N, C]
        import paddle
        import paddle.nn.functional as F
        output = F.softmax(paddle.to_tensor(output), axis=-1).numpy()
        for i in range(N):
            classes = np.argpartition(output[i], -self.top_k)[-self.top_k:]
            classes = classes[np.argsort(-output[i, classes])]
            labels = [self.label_dict[label] for label in classes.tolist()]
            scores = output[i, classes]
        res = {"res" :"class: {} score: {}".format(labels, scores)}
        return res, None, ""

class ppTSNService(WebService):
    def get_pipeline_response(self, read_op):
        ppTSN_op = ppTSNOp(name="ppTSN", input_ops=[read_op])
        return ppTSN_op

pptsn_service = ppTSNService(name="ppTSN")
pptsn_service.prepare_pipeline_config("config.yml")
pptsn_service.run_service()
