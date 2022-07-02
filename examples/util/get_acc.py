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

import sys
import os

total = 0
acc = 0
for line in sys.stdin:
    line = line.strip()
    group = line.split()
    if (float(group[0]) - 0.5) * (float(group[1]) - 0.5) > 0:
        acc += 1
    total += 1

print(float(acc) / float(total))
