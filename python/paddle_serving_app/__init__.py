#   Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
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
from .reader.chinese_bert_reader import ChineseBertReader
from .reader.image_reader import ImageReader, File2Image, URL2Image, Sequential, Normalize, CenterCrop, Resize
from .reader.lac_reader import LACReader
from .reader.senta_reader import SentaReader
from .models import ServingModels
from .local_predict import Debugger
