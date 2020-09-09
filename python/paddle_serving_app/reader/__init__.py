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
from .chinese_bert_reader import ChineseBertReader
from .image_reader import ImageReader, File2Image, URL2Image, Sequential, Normalize
from .image_reader import CenterCrop, Resize, Transpose, Div, RGB2BGR, BGR2RGB, ResizeByFactor
from .image_reader import RCNNPostprocess, SegPostprocess, PadStride
from .image_reader import DBPostProcess, FilterBoxes, GetRotateCropImage, SortedBoxes
from .lac_reader import LACReader
from .senta_reader import SentaReader
from .imdb_reader import IMDBDataset
from .ocr_reader import OCRReader
