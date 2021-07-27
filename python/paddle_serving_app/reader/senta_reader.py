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

import sys
import io
import os


class SentaReader():
    def __init__(self, vocab_path="", max_seq_len=20):
        self.max_seq_len = max_seq_len
        self.word_dict = self.load_vocab(vocab_path)

    def load_vocab(self, vocab_path):
        """
        load the given vocabulary
        """
        if vocab_path == "":
            vocab_path = "senta_vocab.txt"
            if not os.path.exists(vocab_path):
                r = os.system(
                    " wget https://paddle-serving.bj.bcebos.com/reader/senta/senta_vocab.txt --no-check-certificate"
                )

        vocab = {}
        with io.open(vocab_path, 'r', encoding='utf8') as f:
            for line in f:
                if line.strip() not in vocab:
                    data = line.strip().split("\t")
                    if len(data) < 2:
                        word = ""
                        wid = data[0]
                    else:
                        word = data[0]
                        wid = data[1]
                    vocab[word] = int(wid)
        vocab["<unk>"] = len(vocab)
        return vocab

    def process(self, cols):
        unk_id = len(self.word_dict)
        pad_id = 0
        wids = [
            self.word_dict[x] if x in self.word_dict else unk_id for x in cols
        ]
        '''
        seq_len = len(wids)
        if seq_len < self.max_seq_len:
            for i in range(self.max_seq_len - seq_len):
                wids.append(pad_id)
        else:
            wids = wids[:self.max_seq_len]
            seq_len = self.max_seq_len
        '''
        return wids
