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
import subprocess


def update_info(file_name, feature, info):
    new_str = ""
    with open(file_name, "r") as f:
        for line in f.readlines():
            if re.match(feature, line):
                if isinstance(info, str):
                    line = feature + " = \"" + info.strip() + "\"\n"
                else:
                    line = feature + " = \"" + info.decode('utf-8').strip(
                    ) + "\"\n"
            new_str = new_str + line

    with open(file_name, "w") as f:
        f.write(new_str)


if len(sys.argv) > 2 and len(sys.argv[2]) > 0:
    update_info("paddle_serving_server/version.py", "version_suffix",
                sys.argv[2])

package_name = '${SERVER_PACKAGE_NAME}'
if package_name.endswith('gpu'):
    update_info("paddle_serving_server/version.py", "device_type", "1")
elif package_name.endswith('xpu'):
    update_info("paddle_serving_server/version.py", "device_type", "2")
elif package_name.endswith('rocm'):
    update_info("paddle_serving_server/version.py", "device_type", "3")
elif package_name.endswith('npu'):
    update_info("paddle_serving_server/version.py", "device_type", "4")

path = "paddle_serving_" + sys.argv[1]
commit_id = subprocess.check_output(['git', 'rev-parse', 'HEAD'])
update_info(path + "/version.py", "commit_id", commit_id)

update_info(path + "/version.py", "version_tag", "${VERSION_TAG}")
