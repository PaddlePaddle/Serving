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

from paddle_serving_client import Client
import sys
import random
import time
reload(sys)
sys.setdefaultencoding('utf-8')
import os
import io
from bert_reader import BertReader, BertSequenceReader
import paddle.fluid.incubate.data_generator as dg

#import data_generator as dg


class SeqLabelingDataset(dg.MultiSlotDataGenerator):
    def setup(self, dict_path, endpoints):
        self.bert_reader = BertSequenceReader(
            vocab_file="vocab.txt", max_seq_len=128)
        self.endpoint_list = endpoints.split("-")
        self.clients = []
        for ep in self.endpoint_list:
            client = Client()
            client.load_client_config(
                "ernie_crf_client/serving_client_conf.prototxt")
            client.connect([ep])
            self.clients.append(client)
        self.client_idx = 0

    def get_client(self):
        client = self.clients[self.client_idx % len(self.endpoint_list)]
        self.client_idx += 1
        # in case of overflow
        if self.client_idx > 100001:
            self.client_idx = 0
        return client

    def generate_sample(self, line):
        def data_iter():
            #start = time.time()
            group = line.split("\t")
            if len(group) != 2:
                yield None
            feed_dict = self.bert_reader.process_words_and_labels(group[0],
                                                                  group[1])
            labels = feed_dict["label_ids"]
            del feed_dict["label_ids"]
            unpad_seq_len = feed_dict["unpad_seq_lens"][0]
            if unpad_seq_len <= 0:
                yield None
            del feed_dict["unpad_seq_lens"]
            client = self.get_client()
            fetch_map = client.predict(feed=feed_dict, fetch=["crf_decode"])
            targets = fetch_map["crf_decode"][:unpad_seq_len]

            if len(targets) <= 2:
                yield None
            #end = time.time()
            #sys.stderr.write("latency: {}\n".format(end - start))
            # whether to use client to fetch remote data
            feature_name = ["words", "targets"]
            yield ("words", feed_dict["src_ids"][1:unpad_seq_len - 1]), (
                "targets", targets[1:-1])

        return data_iter


if __name__ == "__main__":
    seq_label_dataset = SeqLabelingDataset()
    seq_label_dataset.setup(sys.argv[1], sys.argv[2])
    seq_label_dataset.run_from_stdin()
