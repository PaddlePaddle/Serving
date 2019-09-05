# -*- coding: utf-8 -*-
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
    # conn.putheader('Content-Type', 'application/json') # 如果server版本在r31987后，不需要设置这个。

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
