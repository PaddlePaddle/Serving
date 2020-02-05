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
import sys
import subprocess
from multiprocessing import Pool
import time


def predict(p_id, p_size, data_list):
    client = Client()
    client.load_client_config(conf_file)
    client.connect(["127.0.0.1:8010"])
    result = []
    for line in data_list:
        group = line.strip().split()
        words = [int(x) for x in group[1:int(group[0])]]
        label = [int(group[-1])]
        feed = {"words": words, "label": label}
        fetch = ["acc", "cost", "prediction"]
        fetch_map = client.predict(feed=feed, fetch=fetch)
        #print("{} {}".format(fetch_map["prediction"][1], label[0]))
        result.append([fetch_map["prediction"][1], label[0]])
    return result


def predict_multi_thread(p_num):
    data_list = []
    with open(data_file) as f:
        for line in f.readlines():
            data_list.append(line)
    start = time.time()
    p = Pool(p_num)
    p_size = len(data_list) / p_num
    result_list = []
    for i in range(p_num):
        result_list.append(
            p.apply_async(predict,
                          [i, p_size, data_list[i * p_size:(i + 1) * p_size]]))
    p.close()
    p.join()
    for i in range(p_num):
        result = result_list[i].get()
        for j in result:
            print("{} {}".format(j[0], j[1]))
    cost = time.time() - start
    print("{} threads cost {}".format(p_num, cost))


if __name__ == '__main__':
    conf_file = sys.argv[1]
    data_file = sys.argv[2]
    p_num = int(sys.argv[3])
    predict_multi_thread(p_num)
