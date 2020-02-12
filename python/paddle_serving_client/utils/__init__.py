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
import subprocess
from multiprocessing import Pool

class MultiThreadRunner(object):
    def __init__(self):
        pass

    def run(self, thread_func, thread_num, global_resource):
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
