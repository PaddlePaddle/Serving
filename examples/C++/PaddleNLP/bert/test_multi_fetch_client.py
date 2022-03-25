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
from paddle_serving_app.reader import ChineseBertReader
import sys
import numpy as np

client = Client()
client.load_client_config("./bert_seq32_client/serving_client_conf.prototxt")
client.connect(["127.0.0.1:9292"])

reader = ChineseBertReader({"max_seq_len": 32})
fetch = ["sequence_10", "sequence_12", "pooled_output"]
expected_shape = {
    "sequence_10": (4, 32, 768),
    "sequence_12": (4, 32, 768),
    "pooled_output": (4, 768)
}
batch_size = 4
feed_batch = {}

batch_len = 0
for line in sys.stdin:
    feed = reader.process(line)
    if batch_len == 0:
        for key in feed.keys():
            val_len = len(feed[key])
            feed_batch[key] = np.array(feed[key]).reshape((1, val_len, 1))
        continue
    if len(feed_batch) < batch_size:
        for key in feed.keys():
            np.concatenate([
                feed_batch[key], np.array(feed[key]).reshape((1, val_len, 1))
            ])
    else:
        fetch_map = client.predict(feed=feed_batch, fetch=fetch)
        feed_batch = []
        for var_name in fetch:
            if fetch_map[var_name].shape != expected_shape[var_name]:
                print("fetch var {} shape error.".format(var_name))
                sys.exit(1)
