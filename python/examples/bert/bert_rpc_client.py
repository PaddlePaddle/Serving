# coding:utf-8
# pylint: disable=doc-string-missing
import os
import sys
import numpy as np
import paddlehub as hub
import ujson
import random
import time
from paddlehub.common.logger import logger
import socket
from paddle_serving_client import Client
from paddle_serving_client.utils import MultiThreadRunner
from paddle_serving_client.utils import benchmark_args
from bert_reader import BertReader

args = benchmark_args()

_ver = sys.version_info
is_py2 = (_ver[0] == 2)
is_py3 = (_ver[0] == 3)

if is_py2:
    import httplib
if is_py3:
    import http.client as httplib


class BertService():
    def __init__(self,
                 max_seq_len=128,
                 model_name="bert_uncased_L-12_H-768_A-12",
                 show_ids=False,
                 do_lower_case=True,
                 process_id=0,
                 retry=3):
        self.process_id = process_id
        self.reader_flag = False
        self.batch_size = 0
        self.max_seq_len = max_seq_len
        self.model_name = model_name
        self.show_ids = show_ids
        self.do_lower_case = do_lower_case
        self.retry = retry
        self.pid = os.getpid()
        self.profile = True if ("FLAGS_profile_client" in os.environ and
                                os.environ["FLAGS_profile_client"]) else False
        self.reader = BertReader(vocab_file="vocab.txt", max_seq_len=20)
        self.reader_flag = True

    def load_client(self, config_file, server_addr):
        self.client = Client()
        self.client.load_client_config(config_file)
        self.client.connect(server_addr)

    def run_general(self, text, fetch):
        result = []
        prepro_start = time.time()
        feed = self.reader.process(text)
        if self.profile:
            print("PROFILE\tpid:{}\tbert_pre_0:{} bert_pre_1:{}".format(
                self.pid,
                int(round(prepro_start * 1000000)),
                int(round(prepro_end * 1000000))))
        fetch_map = self.client.predict(feed=feed, fetch=fetch)

        return fetch_map

    def run_batch_general(self, text, fetch):
        self.batch_size = len(text)
        result = []
        prepro_start = time.time()
        feed_batch = []
        for si in range(self.batch_size):
            feed = self.reader.process(text[si])
            feed_batch.append(feed)
        prepro_end = time.time()
        if self.profile:
            print("PROFILE\tpid:{}\tbert_pre_0:{} bert_pre_1:{}".format(
                self.pid,
                int(round(prepro_start * 1000000)),
                int(round(prepro_end * 1000000))))
        fetch_map_batch = self.client.batch_predict(
            feed_batch=feed_batch, fetch=fetch)
        return fetch_map_batch


def single_func(idx, resource):
    bc = BertService(
        model_name='bert_chinese_L-12_H-768_A-12',
        max_seq_len=20,
        show_ids=False,
        do_lower_case=True)
    config_file = './serving_client_conf/serving_client_conf.prototxt'
    fetch = ["pooled_output"]
    server_addr = [resource["endpoint"][idx % len(resource["endpoint"])]]
    bc.load_client(config_file, server_addr)
    batch_size = 1
    use_batch = False if batch_size == 1 else True
    feed_batch = []
    start = time.time()
    fin = open("data-c.txt")
    for line in fin:
        if not use_batch:
            result = bc.run_general(line.strip(), fetch)
        else:
            if len(feed_batch) == batch_size:
                result = bc.run_batch_general(feed_batch, fetch)
                feed_batch = []
            else:
                feed_batch.append(line.strip())
    if use_batch and len(feed_batch) > 0:
        result = bc.run_batch_general(feed_batch, fetch)
        feed_batch = []

    end = time.time()
    return [[end - start]]


if __name__ == '__main__':
    multi_thread_runner = MultiThreadRunner()
    result = multi_thread_runner.run(single_func, args.thread,
                                     {"endpoint": ["127.0.0.1:9292"]})
    print("time cost for each thread {}".format(result))
