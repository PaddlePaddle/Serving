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
        for key in ["alexnet_imagenet"]:
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
