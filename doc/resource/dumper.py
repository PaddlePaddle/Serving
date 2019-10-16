#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Name: dumper.py
#
# Author: wangguibao (wangguibao@baidu.com)
# 
# Usage: python dumper.py --model_path /path/to/model
#
# Dumps CTR model SparseFeatFactors embedding as key-value pairs, then
# convert to Hadoop SequenceFile format
#
# Reference:
# http://wiki.baidu.com/pages/viewpage.action?pageId=651610671
#
import argparse
import logging
import struct
import time
import datetime
import json
from collections import OrderedDict

import numpy as np
import os
import paddle
import paddle.fluid as fluid

from kvtool import SequenceFileWriter
from network_conf import ctr_dnn_model

logging.basicConfig(format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger("fluid")
logger.setLevel(logging.INFO)

NOW_TIMESTAMP = time.time()
NOW_DATETIME = datetime.datetime.now().strftime("%Y%m%d")

def parse_args():
    parser = argparse.ArgumentParser(description="PaddlePaddle DeepFM example")
    parser.add_argument(
        '--model_path',
        type=str,
        required=True,
        help="The path of model parameters file")
    parser.add_argument(
        '--output_data_path',
        type=str,
        default="./ctr_cube/",
        help="The path of dump output")

    parser.add_argument(
        '--embedding_size',
        type=int,
        default=10,
        help="The size for embedding layer (default:10)")
    parser.add_argument(
        '--sparse_feature_dim',
        type=int,
        default=1000001,
        help="The size for embedding layer (default:1000001)")
    parser.add_argument(
        '--batch_size',
        type=int,
        default=1000,
        help="The size of mini-batch (default:1000)")

    return parser.parse_args()

def write_donefile(base_datafile, base_donefile):
    dict = OrderedDict()
    dict["id"] = str(int(NOW_TIMESTAMP))
    dict["key"] = dict["id"]
    dict["input"] = os.path.dirname(base_datafile)

    if not os.access(os.path.dirname(base_donefile), os.F_OK):
        os.makedirs(os.path.dirname(base_donefile))
    donefile_str = ''
    with open(base_donefile, "a") as f:
        jsonstr = json.dumps(dict) + '\n'
        f.write(jsonstr);
    f.close()
def dump():
    args = parse_args()

    output_data_path = os.path.abspath(args.output_data_path)
    base_datafile = output_data_path + "/" + NOW_DATETIME + "/base/feature"
    base_donefile = output_data_path + "/" + "donefile/" + "base.txt"
    patch_datafile = output_data_path + "/" + NOW_DATETIME + "/patch/feature"
    patch_donefile = output_data_path + "/" + "donefile/" + "patch.txt"

    place = fluid.CPUPlace()
    inference_scope = fluid.Scope()

    startup_program = fluid.framework.Program()
    test_program = fluid.framework.Program()
    with fluid.framework.program_guard(test_program, startup_program):
        loss, auc_var, batch_auc_var, _, data_list = ctr_dnn_model(
            args.embedding_size, args.sparse_feature_dim, False)

        exe = fluid.Executor(place)

        feeder = fluid.DataFeeder(feed_list=data_list, place=place)

        fluid.io.load_persistables(
            executor=exe,
            dirname=args.model_path,
            main_program=fluid.default_main_program())

        # Dump embedding
        t = np.array(fluid.global_scope().find_var('SparseFeatFactors').get_tensor())

        if not os.access(os.path.dirname(base_datafile), os.F_OK):
            os.makedirs(os.path.dirname(base_datafile))

        with open(base_datafile, "wb") as f:
            writer = SequenceFileWriter(f)
            for i in range(0, t.shape[0]):
                key_bytes = struct.pack('Q', i)
                row_bytes = struct.pack('%sf' % t.shape[1], *t[i])
                writer.write(key_bytes, row_bytes)
        f.close()

    write_donefile(base_datafile, base_donefile)

if __name__ == '__main__':
    dump()
