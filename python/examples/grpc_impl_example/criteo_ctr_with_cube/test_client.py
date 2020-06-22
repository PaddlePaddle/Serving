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
import sys
import os
import criteo as criteo
import time
from paddle_serving_client.metric import auc
import grpc

client = Client()
client.connect(["127.0.0.1:9292"])

batch = 1
buf_size = 100
dataset = criteo.CriteoDataset()
dataset.setup(1000001)
test_filelists = ["{}/part-0".format(sys.argv[1])]
reader = dataset.infer_reader(test_filelists, batch, buf_size)
label_list = []
prob_list = []
start = time.time()
for ei in range(10000):
    data = reader().next()
    feed_dict = {}
    feed_dict['dense_input'] = data[0][0]
    for i in range(1, 27):
        feed_dict["embedding_{}.tmp_0".format(i - 1)] = data[0][i]
    fetch_map = client.predict(feed=feed_dict, fetch=["prob"])
    if fetch_map["serving_status_code"] == 0:
        prob_list.append(fetch_map['prob'][0][1])
        label_list.append(data[0][-1][0])

print(auc(label_list, prob_list))
end = time.time()
print(end - start)
