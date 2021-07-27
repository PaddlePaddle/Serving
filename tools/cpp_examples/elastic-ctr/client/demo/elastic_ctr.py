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
# pylint: disable=doc-string-missing
from __future__ import print_function
import json
import sys
import os

from elastic_ctr_api import ElasticCTRAPI

BATCH_SIZE = 10
SERVING_IP = "127.0.0.1"
SLOT_CONF_FILE = "./conf/slot.conf"
CTR_EMBEDDING_TABLE_SIZE = 100000001
SLOTS = []


def str2long(str):
    if sys.version_info[0] == 2:
        return long(str)
    elif sys.version_info[0] == 3:
        return int(str)


def tied_rank(x):  # pylint: disable=doc-string-with-all-args, doc-string-with-returns
    """ 
    Computes the tied rank of elements in x.
    This function computes the tied rank of elements in x.
    Parameters
    ----------
    x : list of numbers, numpy array
    Returns
    -------
    score : list of numbers
            The tied rank f each element in x
    """
    sorted_x = sorted(zip(x, range(len(x))))
    r = [0 for k in x]
    cur_val = sorted_x[0][0]
    last_rank = 0
    for i in range(len(sorted_x)):
        if cur_val != sorted_x[i][0]:
            cur_val = sorted_x[i][0]
            for j in range(last_rank, i):
                r[sorted_x[j][1]] = float(last_rank + 1 + i) / 2.0
            last_rank = i
        if i == len(sorted_x) - 1:
            for j in range(last_rank, i + 1):
                r[sorted_x[j][1]] = float(last_rank + i + 2) / 2.0
    return r


def auc(actual, posterior):  # pylint: disable=doc-string-with-all-args, doc-string-with-returns
    """
    Computes the area under the receiver-operater characteristic (AUC)
    This function computes the AUC error metric for binary classification.
    Parameters
    ----------
    actual : list of binary numbers, numpy array
             The ground truth value
    posterior : same type as actual
                Defines a ranking on the binary numbers, from most likely to
                be positive to least likely to be positive.
    Returns
    -------
    score : double
            The mean squared error between actual and posterior
    """
    r = tied_rank(posterior)
    num_positive = len([0 for x in actual if x == 1])
    num_negative = len(actual) - num_positive
    sum_positive = sum([r[i] for i in range(len(r)) if actual[i] == 1])
    auc = ((sum_positive - num_positive * (num_positive + 1) / 2.0) /
           (num_negative * num_positive))
    return auc


def data_reader(data_file, samples, labels):
    if not os.path.exists(data_file):
        print("Path %s not exist" % data_file)
        return -1

    with open(data_file, "r") as f:
        for line in f:
            sample = {}
            line = line.rstrip('\n')
            feature_slots = line.split(' ')
            labels.append(int(feature_slots[1]))
            feature_slots = feature_slots[2:]
            feature_slot_maps = [x.split(':') for x in feature_slots]

            features = [x[0] for x in feature_slot_maps]
            slots = [x[1] for x in feature_slot_maps]

            for i in range(0, len(features)):
                if slots[i] in sample:
                    sample[slots[i]].append(
                        int(features[i]) % CTR_EMBEDDING_TABLE_SIZE)
                else:
                    sample[slots[i]] = [
                        int(features[i]) % CTR_EMBEDDING_TABLE_SIZE
                    ]

            for x in SLOTS:
                if not x in sample:
                    sample[x] = [0]
            samples.append(sample)


if __name__ == "__main__":
    """ main
    """
    if len(sys.argv) != 5:
        print(
            "Usage: python elastic_ctr.py SERVING_IP SERVING_PORT SLOT_CONF_FILE DATA_FILE"
        )
        sys.exit(-1)

    samples = []
    labels = []

    SERVING_IP = sys.argv[1]
    SERVING_PORT = sys.argv[2]
    SLOT_CONF_FILE = sys.argv[3]

    api = ElasticCTRAPI(SERVING_IP, SERVING_PORT)
    ret = api.read_slots_conf(SLOT_CONF_FILE)
    if ret != 0:
        sys.exit(-1)

    ret = data_reader(sys.argv[4], samples, labels)
    correct = 0
    wrong_label_1_count = 0
    result_list = []
    #for i in range(0, len(samples) - BATCH_SIZE, BATCH_SIZE):
    for i in range(0, len(samples), BATCH_SIZE):
        api.clear()
        batch = samples[i:i + BATCH_SIZE]
        instances = []
        for sample in batch:
            instance = api.add_instance()
            if sys.version_info[0] == 2:
                for k, v in sample.iteritems():
                    api.add_slot(instance, k, v)
            elif sys.version_info[0] == 3:
                for k, v in sample.items():
                    api.add_slot(instance, k, v)

        ret = api.inference()
        ret = json.loads(ret)
        predictions = ret["predictions"]

        idx = 0
        for x in predictions:
            result_list.append(x["prob1"])
            if x["prob0"] >= x["prob1"]:
                pred = 0
            else:
                pred = 1

            if labels[i + idx] == pred:
                correct += 1
            else:
                #if labels[i + idx] == 1:
                #    wrong_label_1_count += 1
                #    print("error label=1 count", wrong_label_1_count)
                #print("id=%d predict incorrect: pred=%d label=%d (%f %f)" %
                #      (i + idx, pred, labels[i + idx], x["prob0"], x["prob1"]))
                pass
            idx = idx + 1

    print("auc = ", auc(labels, result_list))
