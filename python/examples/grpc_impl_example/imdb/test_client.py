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
# pylint: disable=doc-string-missing
from paddle_serving_client import MultiLangClient as Client
from paddle_serving_app.reader.imdb_reader import IMDBDataset
import sys
import numpy as np

client = Client()
client.connect(["127.0.0.1:9393"])

# you can define any english sentence or dataset here
# This example reuses imdb reader in training, you
# can define your own data preprocessing easily.
imdb_dataset = IMDBDataset()
imdb_dataset.load_resource('imdb.vocab')

for line in sys.stdin:
    word_ids, label = imdb_dataset.get_words_and_label(line)
    word_len = len(word_ids)
    feed = {
        "words": np.array(word_ids).reshape(word_len, 1),
        "words.lod": [0, word_len]
    }
    fetch = ["prediction"]
    fetch_map = client.predict(feed=feed, fetch=fetch, batch=True)
    if fetch_map["serving_status_code"] == 0:
        print(fetch_map)
    else:
        print(fetch_map["serving_status_code"])
    #print("{} {}".format(fetch_map["prediction"][0], label[0]))
