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

import sys
from paddle_serving_client import Client
from paddle_serving_client.metric import auc
from paddle_serving_client.utils import MultiThreadRunner
import time
from test_bert_client import BertService


def predict(thr_id, resource, batch_size):
    bc = BertService(
        model_name="bert_chinese_L-12_H-768_A-12",
        max_seq_len=20,
        do_lower_case=True)
    bc.load_client(resource["conf_file"], resource["server_endpoint"])
    thread_num = resource["thread_num"]
    file_list = resource["filelist"]
    line_id = 0
    result = []
    label_list = []
    dataset = []
    for fn in file_list:
        fin = open(fn)
        for line in fin:
            if line_id % thread_num == thr_id - 1:
                dataset.append(line.strip())
            line_id += 1
        fin.close()

    start = time.time()
    fetch = ["pooled_output"]
    batch = []
    for inst in dataset:
        if len(batch) < batch_size:
            batch.append([inst])
        else:
            fetch_map_batch = bc.run_batch_general(batch, fetch)
            batch = []
            result.append(fetch_map_batch)
    end = time.time()
    return [result, label_list, [end - start]]


if __name__ == '__main__':
    conf_file = sys.argv[1]
    data_file = sys.argv[2]
    thread_num = sys.argv[3]
    batch_size = sys.ragv[4]
    resource = {}
    resource["conf_file"] = conf_file
    resource["server_endpoint"] = ["127.0.0.1:9293"]
    resource["filelist"] = [data_file]
    resource["thread_num"] = int(thread_num)

    thread_runner = MultiThreadRunner()
    result = thread_runner.run(predict, int(sys.argv[3]), resource, batch_size)

    print("total time {} s".format(sum(result[-1]) / len(result[-1])))
