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
from paddle_serving_client import HttpClient
from paddle_serving_app.reader.imdb_reader import IMDBDataset
import sys
import numpy as np

client = HttpClient()
client.load_client_config(sys.argv[1])
''' 
if you want use GRPC-client, set_use_grpc_client(True)
or you can directly use client.grpc_client_predict(...)
as for HTTP-client,set_use_grpc_client(False)(which is default)
or you can directly use client.http_client_predict(...)
'''
#client.set_use_grpc_client(True)
'''
if you want to enable Encrypt Module,uncommenting the following line
'''
#client.use_key("./key")
'''
if you want to compress,uncommenting the following line
'''
#client.set_response_compress(True)
#client.set_request_compress(True)
'''
we recommend use Proto data format in HTTP-body, set True(which is default)
if you want use JSON data format in HTTP-body, set False
'''
#client.set_http_proto(True)
client.connect(["127.0.0.1:9297"])

# you can define any english sentence or dataset here
# This example reuses imdb reader in training, you
# can define your own data preprocessing easily.
imdb_dataset = IMDBDataset()
imdb_dataset.load_resource(sys.argv[2])

for line in sys.stdin:
    word_ids, label = imdb_dataset.get_words_and_label(line)
    word_len = len(word_ids)
    feed = {
        "words": np.array(word_ids).reshape(word_len, 1),
        "words.lod": [0, word_len]
    }
    #print(feed)
    fetch = ["prediction"]
    fetch_map = client.predict(feed=feed, fetch=fetch, batch=True)
    print(fetch_map)
