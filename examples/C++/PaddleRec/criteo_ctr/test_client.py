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
import sys
import os
import time
from paddle_serving_client.metric import auc
import numpy as np
import sys


class CriteoReader(object):
    def __init__(self, sparse_feature_dim):
        self.cont_min_ = [0, -3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
        self.cont_max_ = [
            20, 600, 100, 50, 64000, 500, 100, 50, 500, 10, 10, 10, 50
        ]
        self.cont_diff_ = [
            20, 603, 100, 50, 64000, 500, 100, 50, 500, 10, 10, 10, 50
        ]
        self.hash_dim_ = sparse_feature_dim
        # here, training data are lines with line_index < train_idx_
        self.train_idx_ = 41256555
        self.continuous_range_ = range(1, 14)
        self.categorical_range_ = range(14, 40)

    def process_line(self, line):
        features = line.rstrip('\n').split('\t')
        dense_feature = []
        sparse_feature = []
        for idx in self.continuous_range_:
            if features[idx] == '':
                dense_feature.append(0.0)
            else:
                dense_feature.append((float(features[idx]) - self.cont_min_[idx - 1]) / \
                                     self.cont_diff_[idx - 1])
        for idx in self.categorical_range_:
            sparse_feature.append(
                [hash(str(idx) + features[idx]) % self.hash_dim_])

        return sparse_feature


py_version = sys.version_info[0]

client = Client()
client.load_client_config(sys.argv[1])
client.connect(["127.0.0.1:9292"])
reader = CriteoReader(1000001)
batch = 1
buf_size = 100
label_list = []
prob_list = []
start = time.time()
f = open(sys.argv[2], 'r')
for ei in range(10):
    data = reader.process_line(f.readline())
    feed_dict = {}
    for i in range(1, 27):
        feed_dict["sparse_{}".format(i - 1)] = np.array(data[i - 1]).reshape(-1)
        feed_dict["sparse_{}.lod".format(i - 1)] = [0, len(data[i - 1])]
    fetch_map = client.predict(feed=feed_dict, fetch=["prob"])
    print(fetch_map)
end = time.time()
f.close()
