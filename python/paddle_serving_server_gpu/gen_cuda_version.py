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
import re
import os

new_str = ""
with open("paddle_serving_server_gpu/version.py", "r") as f:
    for line in f.readlines():
        if re.match("cuda_version", line):
            line = re.sub(r"\d+", sys.argv[1], line)
        new_str = new_str + line

with open("paddle_serving_server_gpu/version.py", "w") as f:
    f.write(new_str)
