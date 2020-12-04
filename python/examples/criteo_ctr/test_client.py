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
import paddle
import sys
import os
import time
import criteo_reader as criteo
from paddle_serving_client.metric import auc
import numpy as np
import sys

py_version = sys.version_info[0]

client = Client()
client.load_client_config(sys.argv[1])
client.connect(["127.0.0.1:9292"])

batch = 1
buf_size = 100
dataset = criteo.CriteoDataset()
dataset.setup(1000001)
test_filelists = [
    "{}/part-%d".format(sys.argv[2]) % x
    for x in range(len(os.listdir(sys.argv[2])))
]
reader = dataset.infer_reader(test_filelists[len(test_filelists) - 40:], batch,
                              buf_size)
label_list = []
prob_list = []
start = time.time()
for ei in range(1000):
    if py_version == 2:
        data = reader().next()
    else:
        data = reader().__next__()
    feed_dict = {}
    for i in range(1, 27):
        feed_dict["sparse_{}".format(i - 1)] = np.array(data[0][i]).reshape(-1)
        feed_dict["sparse_{}.lod".format(i - 1)] = [0, len(data[0][i])]
    fetch_map = client.predict(feed=feed_dict, fetch=["prob"])
end = time.time()
print(end - start)
