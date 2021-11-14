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

from paddle_serving_client import Client
import numpy as np

client = Client()
client.load_client_config('imdb_bow_client_conf/serving_client_conf.prototxt')
client.add_variant("bow", ["127.0.0.1:8000"], 10)
client.add_variant("lstm", ["127.0.0.1:9000"], 90)
client.connect()

print('please wait for about 10s')
with open('processed.data') as f:
    cnt = {"bow": {'acc': 0, 'total': 0}, "lstm": {'acc': 0, 'total': 0}}
    for line in f:
        word_ids, label = line.split(';')
        word_ids = [int(x) for x in word_ids.split(',')]
        word_len = len(word_ids)
        feed = {
            "words": np.array(word_ids).reshape(word_len, 1),
            "words.lod": [0, word_len]
        }
        fetch = ["acc", "cost", "prediction"]
        [fetch_map, tag] = client.predict(
            feed=feed, fetch=fetch, need_variant_tag=True, batch=True)
        if (float(fetch_map["prediction"][0][1]) - 0.5) * (float(label[0]) - 0.5
                                                           ) > 0:
            cnt[tag]['acc'] += 1
        cnt[tag]['total'] += 1

    for tag, data in cnt.items():
        print('[{}](total: {}) acc: {}'.format(tag, data[
            'total'], float(data['acc']) / float(data['total'])))
