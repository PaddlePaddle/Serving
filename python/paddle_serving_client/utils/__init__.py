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
import os
import sys
import subprocess
import argparse
from multiprocessing import Pool


def benchmark_args():
    parser = argparse.ArgumentParser("benchmark")
    parser.add_argument("--thread", type=int, default=10, help="concurrecy")
    parser.add_argument(
        "--model", type=str, default="", help="model for evaluation")
    parser.add_argument(
        "--endpoint",
        type=str,
        default="127.0.0.1:9292",
        help="endpoint of server")
    parser.add_argument(
        "--request", type=str, default="rpc", help="mode of service")
    parser.add_argument("--batch_size", type=int, default=1, help="batch size")
    return parser.parse_args()


class MultiThreadRunner(object):
    def __init__(self):
        pass

    def run(self, thread_func, thread_num, global_resource):
        os.environ["http_proxy"] = ""
        os.environ["https_proxy"] = ""
        p = Pool(thread_num)
        result_list = []
        for i in range(thread_num):
            result_list.append(
                p.apply_async(thread_func, [i + 1, global_resource]))
        p.close()
        p.join()
        return_result = result_list[0].get()
        for i in range(1, thread_num, 1):
            tmp_result = result_list[i].get()
            for i, item in enumerate(tmp_result):
                return_result[i].extend(tmp_result[i])
        return return_result
