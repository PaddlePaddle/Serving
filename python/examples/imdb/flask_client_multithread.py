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
import subprocess
from multiprocessing import Pool
import time
import re
import json
import os
import subprocess as sp

class IMDBDataset():
    def load_resource(self, dictfile):
        self._vocab = {}
        wid = 0
        with open(dictfile) as f:
            for line in f:
                self._vocab[line.strip()] = wid
                wid += 1
        self._unk_id = len(self._vocab)
        self._pattern = re.compile(r'(;|,|\.|\?|!|\s|\(|\))')
        self.return_value = ("words", [1, 2, 3, 4, 5, 6]), ("label", [0])

    def get_words_and_label(self, line):
        send = '|'.join(line.split('|')[:-1]).lower().replace("<br />",
                                                              " ").strip()
        label = [int(line.split('|')[-1])]

        words = [x for x in self._pattern.split(send) if x and x != " "]
        feas = [
            self._vocab[x] if x in self._vocab else self._unk_id for x in words
        ]
        return feas, label

    def infer_reader(self, infer_filelist, batch, buf_size):
        def local_iter():
            for fname in infer_filelist:
                with open(fname, "r") as fin:
                    for line in fin:
                        feas, label = self.get_words_and_label(line)
                        yield feas, label
        import paddle
        batch_iter = paddle.batch(
            paddle.reader.shuffle(local_iter, buf_size=buf_size),
            batch_size=batch)
        return batch_iter


def predict_http(p_id, p_size, data_file):
    start = time.time() 
    ip_api_addr = 'http://127.0.0.1:9294/PaddleServing/v1.0/prediction'
    dataset = IMDBDataset()
    dataset.load_resource("imdb.vocab")
    result = [] 
    for line in data_file:
        words, label = dataset.get_words_and_label(line)
        feed = {"words": words, "fetch":["prediction"]}
        command = 'curl -H "Content-Type:application/json" -X POST -d \'' + json.dumps(feed) + '\' ' + ip_api_addr
        out = os.popen(command).read()
        result.append(json.loads(out)['prediction'])
    return result

def predict_multi_thread(p_num):
    data_list = []
    with open(data_file) as f:
        for line in f.readlines():
            data_list.append(line)
    start = time.time()
    p = Pool(p_num)
    p_size = len(data_list) / p_num
    result_list = []
    for i in range(p_num):
        result_list.append(
            p.apply_async(predict_http,
                          [i, p_size, data_list[i * p_size:(i + 1) * p_size]]))
    p.close()
    p.join()
    for i in range(p_num):
        result = result_list[i].get()
    cost = time.time() - start
    print("{} threads_cost {}".format(p_num, cost))


if __name__ == '__main__':
    conf_file = sys.argv[1]
    data_file = sys.argv[2]
    p_num = int(sys.argv[3])
    predict_multi_thread(p_num)
