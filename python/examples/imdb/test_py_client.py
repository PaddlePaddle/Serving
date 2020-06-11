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
from paddle_serving_client.pyclient import PyClient
import numpy as np
from paddle_serving_app.reader import IMDBDataset
from line_profiler import LineProfiler

client = PyClient()
client.connect('localhost:8080')

lp = LineProfiler()
lp_wrapper = lp(client.predict)

words = 'i am very sad | 0'
imdb_dataset = IMDBDataset()
imdb_dataset.load_resource('imdb.vocab')

for i in range(1):
    word_ids, label = imdb_dataset.get_words_and_label(words)
    fetch_map = lp_wrapper(
        feed={"words": word_ids}, fetch=["combined_prediction"])
    print(fetch_map)

#lp.print_stats()
