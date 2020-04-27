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
        #senta
        for key in [
                "senta_bilstm", "senta_bow", "senta_cnn", "senta_gru",
                "senta_lstm"
        ]:
            self.model_dict[
                key] = "https://paddle-serving.bj.bcebos.com/paddle_hub_models/text/SentimentAnalysis/" + key + ".tar.gz"
        #image classification
        for key in [
                "alexnet_imagenet",
                "darknet53-imagenet",
                "densenet121_imagenet",
                "densenet161_imagenet",
                "densenet169_imagenet",
                "densenet201_imagenet",
                "densenet264_imagenet"
                "dpn107_imagenet",
                "dpn131_imagenet",
                "dpn68_imagenet",
                "dpn92_imagenet",
                "dpn98_imagenet",
                "efficientnetb0_imagenet",
                "efficientnetb1_imagenet",
                "efficientnetb2_imagenet",
                "efficientnetb3_imagenet",
                "efficientnetb4_imagenet",
                "efficientnetb5_imagenet",
                "efficientnetb6_imagenet",
                "googlenet_imagenet",
                "inception_v4_imagenet",
                "inception_v2_imagenet",
                "nasnet_imagenet",
                "pnasnet_imagenet",
                "resnet_v2_101_imagenet",
                "resnet_v2_151_imagenet",
                "resnet_v2_18_imagenet",
                "resnet_v2_34_imagenet",
                " resnet_v2_50_imagenet",
                "resnext101_32x16d_wsl",
                "resnext101_32x32d_wsl",
                "resnext101_32x48d_wsl",
                "resnext101_32x8d_wsl",
                "resnext101_32x4d_imagenet",
                "resnext101_64x4d_imagenet",
                "resnext101_vd_32x4d_imagenet",
                "resnext101_vd_64x4d_imagenet",
                "resnext152_64x4d_imagenet",
                "resnext152_vd_64x4d_imagenet",
                "resnext50_64x4d_imagenet",
                "resnext50_vd_32x4d_imagenet",
                "resnext50_vd_64x4d_imagenet",
                "se_resnext101_32x4d_imagenet",
                "se_resnext50_32x4d_imagenet",
                "shufflenet_v2_imagenet",
                "vgg11_imagenet",
                "vgg13_imagenet",
                "vgg16_imagenet",
                "vgg19_imagenet",
                "xception65_imagenet",
                "xception71_imagenet",
        ]:
            self.model_dict[
                key] = "https://paddle-serving.bj.bcebos.com/paddle_hub_models/image/ImageClassification/" + key + ".tar.gz"

    def get_model_list(self):
        return (self.model_dict.keys())

    def download(self, model_name):
        if model_name in self.model_dict:
            url = self.model_dict[model_name]
            r = os.system('wget ' + url + ' --no-check-certificate')


if __name__ == "__main__":
    models = ServingModels()
    print(models.get_model_list())
