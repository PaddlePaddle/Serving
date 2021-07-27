#   Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
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

import json
import httplib
import sys
import os

BATCH_SIZE = 10


def data_reader(data_file, samples, labels):
    if not os.path.exists(data_file):
        print "Path %s not exist" % data_file
        return -1

    with open(data_file, "r") as f:
        for line in f:
            line = line.replace('(', ' ')
            line = line.replace(')', ' ')
            line = line.replace('[', ' ')
            line = line.replace(']', ' ')
            ids = line.split(',')
            ids = [int(x) for x in ids]
            label = ids[-1]
            ids = ids[0:-1]
            samples.append(ids)
            labels.append(label)


if __name__ == "__main__":
    """ main
    """
    if len(sys.argv) != 2:
        print "Usage: python text_classification.py DATA_FILE"
        sys.exit(-1)

    samples = []
    labels = []
    ret = data_reader(sys.argv[1], samples, labels)

    conn = httplib.HTTPConnection("127.0.0.1", 8010)

    for i in range(0, len(samples) - BATCH_SIZE, BATCH_SIZE):
        batch = samples[i:i + BATCH_SIZE]
        ids = []
        for x in batch:
            ids.append({"ids": x})
        ids = {"instances": ids}

        request_json = json.dumps(ids)

        try:
            conn.request('POST', "/TextClassificationService/inference",
                         request_json, {"Content-Type": "application/json"})
            response = conn.getresponse()
            print response.read()
        except httplib.HTTPException as e:
            print e.reason
