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
import os
from collections import OrderedDict


class ServingModels(object):
    def __init__(self):
        self.model_dict = OrderedDict()
        self.model_dict[
            "SentimentAnalysis"] = ["senta_bilstm", "senta_bow", "senta_cnn"]
        self.model_dict["SemanticRepresentation"] = ["ernie"]
        self.model_dict["ChineseWordSegmentation"] = ["lac"]
        self.model_dict[
            "ObjectDetection"] = ["faster_rcnn", "yolov4", "blazeface"]
        self.model_dict["ImageSegmentation"] = [
            "unet", "deeplabv3", "deeplabv3+cityscapes"
        ]
        self.model_dict["ImageClassification"] = [
            "resnet_v2_50_imagenet", "mobilenet_v2_imagenet"
        ]
        #self.model_dict["TextDetection"] = ["ocr_det"]
        self.model_dict["OCR"] = ["ocr_rec", "ocr_det"]

        image_class_url = "https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ImageClassification/"
        image_seg_url = "https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ImageSegmentation/"
        object_detection_url = "https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ObjectDetection/"
        #ocr_url = "https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/OCR/"
        ocr_url = "https://paddle-serving.bj.bcebos.com/ocr_v2/"
        senta_url = "https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/SentimentAnalysis/"
        semantic_url = "https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/SemanticModel/"
        wordseg_url = "https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/LexicalAnalysis/"
        #ocr_det_url = "https://paddle-serving.bj.bcebos.com/ocr/"

        self.url_dict = {}

        def pack_url(model_dict, key, url):
            for i, value in enumerate(model_dict[key]):
                self.url_dict[model_dict[key][i]] = url + model_dict[key][
                    i] + ".tar.gz"

        pack_url(self.model_dict, "SentimentAnalysis", senta_url)
        pack_url(self.model_dict, "SemanticRepresentation", semantic_url)
        pack_url(self.model_dict, "ChineseWordSegmentation", wordseg_url)
        pack_url(self.model_dict, "ObjectDetection", object_detection_url)
        pack_url(self.model_dict, "ImageSegmentation", image_seg_url)
        pack_url(self.model_dict, "ImageClassification", image_class_url)
        pack_url(self.model_dict, "OCR", ocr_url)
        #pack_url(self.model_dict, "TextDetection", ocr_det_url)

    def get_model_list(self):
        return self.model_dict

    def download(self, model_name):
        if model_name in self.url_dict:
            url = self.url_dict[model_name]
            r = os.system('wget ' + url + ' --no-check-certificate')

    def get_tutorial(self, model_name):
        if model_name in self.tutorial_url:
            return "Tutorial of {} to be added".format(model_name)


if __name__ == "__main__":
    models = ServingModels()
    print(models.get_model_list())
