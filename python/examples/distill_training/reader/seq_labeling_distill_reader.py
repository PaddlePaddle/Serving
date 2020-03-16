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
from multiprocessing import Process, Queue
from threading import Thread
import paddle.fluid.incubate.data_generator as dg


def dispatcher(inputqueues, fin):
    idx = 0
    for line in fin:
        inputqueues[idx % len(inputqueues)].put(line)
        idx += 1
    for queue in inputqueues:
        queue.put(None)


def client_producer(inputqueue, outputqueue, endpoint, bert_reader):
    client = Client()
    client.load_client_config("ernie_crf_client/serving_client_conf.prototxt")
    client.connect([endpoint])
    while True:
        line = inputqueue.get()
        if line == None:
            break
        group = line.split("\t")
        feed_dict = bert_reader.process_words_and_labels(group[0], group[1])
        labels = feed_dict["label_ids"]
        del feed_dict["label_ids"]
        unpad_seq_len = feed_dict["unpad_seq_lens"][0]
        if unpad_seq_len <= 0:
            continue
        del feed_dict["unpad_seq_lens"]
        fetch_map = client.predict(feed=feed_dict, fetch=["crf_decode"])
        targets = fetch_map["crf_decode"][:unpad_seq_len]
        outputqueue.put(
            [feed_dict["src_ids"][1:unpad_seq_len - 1], targets[1:-1]])
    outputqueue.put(None)


class SeqLabelingDataset(dg.MultiSlotDataGenerator):
    def setup(self, endpoints):
        self.bert_reader = BertSequenceReader(
            vocab_file="vocab.txt", max_seq_len=128)
        self.endpoint_list = endpoints.split("-")
        self.input_queues = []
        for i in range(len(self.endpoint_list)):
            self.input_queues.append(Queue())
        self.result_queue = Queue()
        self.producers = [
            Process(
                target=client_producer,
                args=(
                    self.input_queues[i],
                    self.result_queue,
                    self.endpoint_list[i],
                    self.bert_reader, ))
            for i in range(len(self.endpoint_list))
        ]

        for p in self.producers:
            p.start()

    def run_from_stdin(self):
        dispatcher_thread = Thread(
            target=dispatcher, args=(
                self.input_queues,
                sys.stdin, ))
        dispatcher_thread.start()

        while True:
            result = self.result_queue.get()
            if result == None:
                break
            sample = ("words", result[0]), ("targets", result[1])
            sys.stdout.write(self._gen_str(sample))
        p.join()
        for pro in self.producers:
            pro.join()
        dispatcher_thread.join()


if __name__ == "__main__":
    seq_label_dataset = SeqLabelingDataset()
    seq_label_dataset.setup(sys.argv[1])
    seq_label_dataset.run_from_stdin()
