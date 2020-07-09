# -*- coding: utf-8 -*-
#
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
import criteo as criteo
import time
from paddle_serving_client.utils import MultiThreadRunner
from paddle_serving_client.utils import benchmark_args
from paddle_serving_client.metric import auc

py_version = sys.version_info[0]
args = benchmark_args()


def single_func(idx, resource):
    client = Client()
    print([resource["endpoint"][idx % len(resource["endpoint"])]])
    client.load_client_config('ctr_client_conf/serving_client_conf.prototxt')
    client.connect(['127.0.0.1:9292'])
    batch = 1
    buf_size = 100
    dataset = criteo.CriteoDataset()
    dataset.setup(1000001)
    test_filelists = [
        "./raw_data/part-%d" % x for x in range(len(os.listdir("./raw_data")))
    ]
    reader = dataset.infer_reader(test_filelists[len(test_filelists) - 40:],
                                  batch, buf_size)
    if args.request == "rpc":
        fetch = ["prob"]
        start = time.time()
        itr = 1000
        for ei in range(itr):
            if args.batch_size > 0:
                feed_batch = []
                for bi in range(args.batch_size):
                    if py_version == 2:
                        data = reader().next()
                    else:
                        data = reader().__next__()
                    feed_dict = {}
                    feed_dict['dense_input'] = data[0][0]
                    for i in range(1, 27):
                        feed_dict["embedding_{}.tmp_0".format(i - 1)] = data[0][
                            i]
                    feed_batch.append(feed_dict)
                result = client.predict(feed=feed_batch, fetch=fetch)
            else:
                print("unsupport batch size {}".format(args.batch_size))

    elif args.request == "http":
        raise ("Not support http service.")
    end = time.time()
    qps = itr * args.batch_size / (end - start)
    return [[end - start, qps]]


if __name__ == '__main__':
    multi_thread_runner = MultiThreadRunner()
    endpoint_list = ["127.0.0.1:9292"]
    #result = single_func(0, {"endpoint": endpoint_list})
    start = time.time()
    result = multi_thread_runner.run(single_func, args.thread,
                                     {"endpoint": endpoint_list})
    end = time.time()
    total_cost = end - start
    avg_cost = 0
    qps = 0
    for i in range(args.thread):
        avg_cost += result[0][i * 2 + 0]
        qps += result[0][i * 2 + 1]
    avg_cost = avg_cost / args.thread
    print("total cost: {}".format(total_cost))
    print("average total cost {} s.".format(avg_cost))
    print("qps {} ins/s".format(qps))
