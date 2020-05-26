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
from paddle_serving_client.pyclient import PyClient
import numpy as np

from line_profiler import LineProfiler

client = PyClient()
client.connect('localhost:8080')

x = np.array(
    [
        0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584,
        0.6283, 0.4919, 0.1856, 0.0795, -0.0332
    ],
    dtype='float')

lp = LineProfiler()
lp_wrapper = lp(client.predict)

for i in range(3):
    fetch_map = lp_wrapper(
        feed={"x": x}, fetch_with_type={"combine_op_output": "float"})
    # fetch_map = client.predict(
    # feed={"x": x}, fetch_with_type={"combine_op_output": "float"})
    print(fetch_map)

lp.print_stats()
