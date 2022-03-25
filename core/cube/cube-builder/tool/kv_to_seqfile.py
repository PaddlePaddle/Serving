# Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License

import struct
import time
import datetime
import json
from collections import OrderedDict
import os
from kvtool import SequenceFileWriter

NOW_TIMESTAMP = time.time()
NOW_DATETIME = datetime.datetime.now().strftime("%Y%m%d%H%M%S")
DATA_PATH = "./data/"
BASE_DATAFILE = DATA_PATH + NOW_DATETIME + "/base/feature"
BASE_DONEFILE = DATA_PATH + "donefile/base.txt"
PATCH_DATAFILE = "./donefile/" + NOW_DATETIME + "/patch/feature"
PATCH_DONEFILE = DATA_PATH + "donefile/patch.txt"
SOURCE_FILE = './source/file.txt'


def write_donefile():
    dict = OrderedDict()
    dict["id"] = str(int(NOW_TIMESTAMP))
    dict["key"] = dict["id"]
    dict["input"] = os.path.abspath(os.path.dirname(BASE_DATAFILE))
    if not os.access(os.path.dirname(BASE_DONEFILE), os.F_OK):
        os.makedirs(os.path.dirname(BASE_DONEFILE))
    with open(BASE_DONEFILE, "a+") as f:
        f.write(json.dumps(dict) + '\n')
    f.close()


def kv_to_seqfile():
    if not os.access(os.path.dirname(BASE_DATAFILE), os.F_OK):
        os.makedirs(os.path.dirname(BASE_DATAFILE))
    with open(BASE_DATAFILE, "wb") as f:
        writer = SequenceFileWriter(f)
        res = []
        fp = open(SOURCE_FILE, 'r')
        try:
            lines = fp.readlines()
        finally:
            fp.close()
        for line in lines:
            line_list = line.split()
            if len(line_list) < 1:
                continue
            key = int(line_list[0])
            show = int(line_list[1])
            click = int(line_list[2])
            values = [float(x) for x in line_list[3:]]

           # str(line_list[1]).replace('\n', '')
            res.append(dict)
            key_bytes = struct.pack('Q', key)
            row_bytes = ""
            for v in values:
                row_bytes += struct.pack('f', v)
            print key, ':', values, '->', key_bytes, ':', row_bytes
            writer.write(key_bytes, row_bytes)
    f.close()
    write_donefile()


if __name__ == '__main__':
    """
    tran kv to sequence file and auto create donefile file, you can modify source file addr,output addr in line 25-32
    """
    kv_to_seqfile()
