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
from paddle_serving_client import Client
from imdb_reader import IMDBDataset
import sys

client = Client()
client.load_client_config(sys.argv[1])
client.connect(["127.0.0.1:9292"])

# you can define any english sentence or dataset here
# This example reuses imdb reader in training, you
# can define your own data preprocessing easily.
imdb_dataset = IMDBDataset()
imdb_dataset.load_resource(sys.argv[2])

for line in sys.stdin:
    word_ids, label = imdb_dataset.get_words_and_label(line)
    feed = {"words": word_ids}
    fetch = ["acc", "cost", "prediction"]
    fetch_map = client.predict(feed=feed, fetch=fetch)
    print("{} {}".format(fetch_map["prediction"][1], label[0]))
