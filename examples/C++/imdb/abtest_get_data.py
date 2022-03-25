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

from paddle_serving_app.reader.imdb_reader import IMDBDataset
imdb_dataset = IMDBDataset()
imdb_dataset.load_resource('imdb.vocab')

with open('test_data/part-0') as fin:
    with open('processed.data', 'w') as fout:
        for line in fin:
            word_ids, label = imdb_dataset.get_words_and_label(line)
            fout.write("{};{}\n".format(','.join([str(x) for x in word_ids]),
                                        label[0]))
