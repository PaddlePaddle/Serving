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

BATCH_SIZE = 3
SERVING_IP = "127.0.0.1"
SLOT_CONF_FILE = "./conf/slot.conf"
CTR_EMBEDDING_TABLE_SIZE = 400000001
SLOTS = []


def data_reader(data_file, samples, labels):
    if not os.path.exists(data_file):
        print "Path %s not exist" % data_file
        return -1

    with open(data_file, "r") as f:
        for line in f:
            sample = {}
            line = line.rstrip('\n')
            feature_slots = line.split(' ')
            feature_slots = feature_slots[2:]
            feature_slot_maps = [x.split(':') for x in feature_slots]

            features = [x[0] for x in feature_slot_maps]
            slots = [x[1] for x in feature_slot_maps]

            for i in range(0, len(features)):
                if slots[i] in sample:
                    sample[slots[i]] = [sample[slots[i]] + long(features[i])]
                else:
                    sample[slots[
                        i]] = [long(features[i]) % CTR_EMBEDDING_TABLE_SIZE]

            for x in SLOTS:
                if not x in sample:
                    sample[x] = [0]
            samples.append(sample)


def read_slots_conf(slots_conf_file, slots):
    if not os.path.exists(slots_conf_file):
        print "Path %s not exist" % sltos_conf_file
        return -1
    with open(slots_conf_file, "r") as f:
        for line in f:
            slots.append(line.rstrip('\n'))
    print slots
    return 0


if __name__ == "__main__":
    """ main
    """
    if len(sys.argv) != 4:
        print "Usage: python elastic_ctr.py SERVING_IP SLOT_CONF_FILE DATA_FILE"
        sys.exit(-1)

    samples = []
    labels = []

    SERVING_IP = sys.argv[1]
    SLOT_CONF_FILE = sys.argv[2]

    ret = read_slots_conf(SLOT_CONF_FILE, SLOTS)
    if ret != 0:
        sys.exit(-1)
    print SLOTS

    ret = data_reader(sys.argv[3], samples, labels)

    conn = httplib.HTTPConnection(SERVING_IP, 8010)

    for i in range(0, len(samples) - BATCH_SIZE, BATCH_SIZE):
        batch = samples[i:i + BATCH_SIZE]
        instances = []
        for sample in batch:
            instance = []
            kv = []
            for k, v in sample.iteritems():
                kv += [{"slot_name": k, "feasigns": v}]
            print kv
            instance = [{"slots": kv}]
            instances += instance
        req = {"instances": instances}

        request_json = json.dumps(req)
        print request_json

        try:
            conn.request('POST', "/ElasticCTRPredictionService/inference",
                         request_json, {"Content-Type": "application/json"})
            response = conn.getresponse()
            print response.read()
        except httplib.HTTPException as e:
            print e.reason
        sys.exit(0)
