#   Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
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

import os
import re
import sys


def ReadMarkDown(file):
    folder = 'test'
    os.system('rm -rf ' + folder + ' && mkdir -p ' + folder)
    with open(file, 'r') as f:
        lines = f.readlines()
    for i, line in enumerate(lines):
        if '[//file]:#' in line:
            filename = line[10:].strip()
            GetCodeFile(lines, i, os.path.join(folder, filename))
        if '<!--' in line:
            filename = 'start.sh'
            GetTestFile(lines, i, os.path.join(folder, filename))


def GetCodeFile(lines, i, filename):
    if '```' not in lines[i + 1]:
        raise ValueError(
            'Syntax Error, code block should be tightly followed by "[//file]:#" '
        )
    i += 2
    code = ''
    while True:
        if '```' in lines[i]:
            break
        code += lines[i]
        i += 1
    with open(filename, 'w+') as f:
        f.write(code)


def GetTestFile(lines, i, filename):
    i += 1
    code = ''
    while True:
        if '-->' in lines[i]:
            break
        code += lines[i]
        i += 1
    with open(filename, 'w+') as f:
        f.write(code)


def RunTest():
    folder = 'test'
    os.system('cd ' + folder + ' && sh start.sh')
    os.system('cd .. && rm -rf ' + folder)


if __name__ == '__main__':
    ReadMarkDown(os.path.join(sys.argv[1], sys.argv[2]))
    RunTest()
