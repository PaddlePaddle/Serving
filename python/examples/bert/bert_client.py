# coding:utf-8
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
        self.profile = True if ("FLAGS_profile_client" in os.environ and
                                os.environ["FLAGS_profile_client"]) else False

        module = hub.Module(name=self.model_name)
        inputs, outputs, program = module.context(
            trainable=True, max_seq_len=self.max_seq_len)
        input_ids = inputs["input_ids"]
        position_ids = inputs["position_ids"]
        segment_ids = inputs["segment_ids"]
        input_mask = inputs["input_mask"]
        self.reader = hub.reader.ClassifyReader(
            vocab_path=module.get_vocab_path(),
            dataset=None,
            max_seq_len=self.max_seq_len,
            do_lower_case=self.do_lower_case)
        self.reader_flag = True

    def load_client(self, config_file, server_addr):
        self.client = Client()
        self.client.load_client_config(config_file)
        self.client.connect(server_addr)

    def run_general(self, text, fetch):
        self.batch_size = len(text)
        data_generator = self.reader.data_generator(
            batch_size=self.batch_size, phase='predict', data=text)
        result = []
        prepro_start = time.time()
        for run_step, batch in enumerate(data_generator(), start=1):
            token_list = batch[0][0].reshape(-1).tolist()
            pos_list = batch[0][1].reshape(-1).tolist()
            sent_list = batch[0][2].reshape(-1).tolist()
            mask_list = batch[0][3].reshape(-1).tolist()
            for si in range(self.batch_size):
                feed = {
                    "input_ids": token_list,
                    "position_ids": pos_list,
                    "segment_ids": sent_list,
                    "input_mask": mask_list
                }
                prepro_end = time.time()
                if self.profile:
                    print("PROFILE\tbert_pre_0:{} bert_pre_1:{}".format(
                        int(round(prepro_start * 1000000)),
                        int(round(prepro_end * 1000000))))
                fetch_map = self.client.predict(feed=feed, fetch=fetch)

        return fetch_map

    def run_batch_general(self, text, fetch):
        self.batch_size = len(text)
        data_generator = self.reader.data_generator(
            batch_size=self.batch_size, phase='predict', data=text)
        result = []
        prepro_start = time.time()
        for run_step, batch in enumerate(data_generator(), start=1):
            token_list = batch[0][0].reshape(-1).tolist()
            pos_list = batch[0][1].reshape(-1).tolist()
            sent_list = batch[0][2].reshape(-1).tolist()
            mask_list = batch[0][3].reshape(-1).tolist()
            feed_batch = []
            for si in range(self.batch_size):
                feed = {
                    "input_ids": token_list[si * self.max_seq_len:(si + 1) *
                                            self.max_seq_len],
                    "position_ids":
                    pos_list[si * self.max_seq_len:(si + 1) * self.max_seq_len],
                    "segment_ids": sent_list[si * self.max_seq_len:(si + 1) *
                                             self.max_seq_len],
                    "input_mask":
                    mask_list[si * self.max_seq_len:(si + 1) * self.max_seq_len]
                }
                feed_batch.append(feed)
            prepro_end = time.time()
            if self.profile:
                print("PROFILE\tbert_pre_0:{} bert_pre_1:{}".format(
                    int(round(prepro_start * 1000000)),
                    int(round(prepro_end * 1000000))))
            fetch_map_batch = self.client.batch_predict(
                feed_batch=feed_batch, fetch=fetch)
        return fetch_map_batch


def test():
    bc = BertService(
        model_name='bert_chinese_L-12_H-768_A-12',
        max_seq_len=20,
        show_ids=False,
        do_lower_case=True)
    server_addr = ["127.0.0.1:9292"]
    config_file = './serving_client_conf/serving_client_conf.prototxt'
    fetch = ["pooled_output"]
    bc.load_client(config_file, server_addr)
    batch_size = 1
    batch = []
    for line in sys.stdin:
        if batch_size == 1:
            result = bc.run_general([[line.strip()]], fetch)
            print(result)
        else:
            if len(batch) < batch_size:
                batch.append([line.strip()])
            else:
                result = bc.run_batch_general(batch, fetch)
                batch = []
                for r in result:
                    print(r)
    if len(batch) > 0:
        result = bc.run_batch_general(batch, fetch)
        batch = []
        for r in result:
            print(r)


if __name__ == '__main__':
    test()
