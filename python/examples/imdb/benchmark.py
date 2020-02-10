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

def predict(thr_id, resource):
    client = Client()
    client.load_client_config(resource["conf_file"])
    client.connect(resource["server_endpoint"])
    thread_num = resource["thread_num"]
    file_list = resource["filelist"]
    line_id = 0
    prob = []
    label_list = []
    dataset = []
    for fn in file_list:
        fin = open(fn)
        for line in fin:
            if line_id % thread_num == thr_id - 1:
                group = line.strip().split()
                words = [int(x) for x in group[1:int(group[0])]]
                label = [int(group[-1])]
                feed = {"words": words, "label": label}
                dataset.append(feed)
            line_id += 1
        fin.close()

    start = time.time()
    fetch = ["acc", "cost", "prediction"]
    infer_time_list = []
    for inst in dataset:
        fetch_map = client.predict(feed=inst, fetch=fetch, debug=True)
        prob.append(fetch_map["prediction"][1])
        label_list.append(label[0])
        infer_time_list.append(fetch_map["infer_time"])
    end = time.time()
    client.release()
    return [prob, label_list, [sum(infer_time_list)], [end - start]]

if __name__ == '__main__':
    conf_file = sys.argv[1]
    data_file = sys.argv[2]
    resource = {}
    resource["conf_file"] = conf_file
    resource["server_endpoint"] = ["127.0.0.1:9292"]
    resource["filelist"] = [data_file]
    resource["thread_num"] = int(sys.argv[3])

    thread_runner = MultiThreadRunner()
    result = thread_runner.run(predict, int(sys.argv[3]), resource)

    print("{}\t{}".format(sys.argv[3], sum(result[-1]) / len(result[-1])))
    print("{}\t{}".format(sys.argv[3], sum(result[2]) / 1000.0 / 1000.0 / len(result[2])))
