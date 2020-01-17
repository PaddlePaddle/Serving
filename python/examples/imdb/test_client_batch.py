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

from paddle_serving import Client
import sys
import subprocess
from multiprocessing import Pool
import time


def batch_predict(batch_size=4):
    client = Client()
    client.load_client_config(conf_file)
    client.connect(["127.0.0.1:8010"])
    start = time.time()
    feed_batch = []
    for line in sys.stdin:
        group = line.strip().split()
        words = [int(x) for x in group[1:int(group[0])]]
        label = [int(group[-1])]
        feed = {"words": words, "label": label}
        fetch = ["acc", "cost", "prediction"]
        feed_batch.append(feed)
        if len(feed_batch) == batch_size:
            fetch_batch = client.batch_predict(
                feed_batch=feed_batch, fetch=fetch)
            for i in range(batch_size):
                print("{} {}".format(fetch_batch[i]["prediction"][1],
                                     feed_batch[i]["label"][0]))
            feed_batch = []
    cost = time.time() - start
    print("total cost : {}".format(cost))
    print(time.time())


if __name__ == '__main__':
    conf_file = sys.argv[1]
    batch_size = int(sys.argv[2])
    batch_predict(batch_size)
