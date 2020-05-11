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
py_version = sys.version_info[0]
if py_version == 2:
    reload(sys)
    sys.setdefaultencoding('utf-8')
import os
import io


def load_kv_dict(dict_path,
                 reverse=False,
                 delimiter="\t",
                 key_func=None,
                 value_func=None):
    result_dict = {}
    for line in io.open(dict_path, "r", encoding="utf8"):
        terms = line.strip("\n").split(delimiter)
        if len(terms) != 2:
            continue
        if reverse:
            value, key = terms
        else:
            key, value = terms
        if key in result_dict:
            raise KeyError("key duplicated with [%s]" % (key))
        if key_func:
            key = key_func(key)
        if value_func:
            value = value_func(value)
        result_dict[key] = value
    return result_dict


class LACReader(object):
    """data reader"""

    def __init__(self, dict_folder):
        # read dict
        #basepath = os.path.abspath(__file__)
        #folder = os.path.dirname(basepath)
        word_dict_path = os.path.join(dict_folder, "word.dic")
        label_dict_path = os.path.join(dict_folder, "tag.dic")
        self.word2id_dict = load_kv_dict(
            word_dict_path, reverse=True, value_func=int)
        self.id2word_dict = load_kv_dict(word_dict_path)
        self.label2id_dict = load_kv_dict(
            label_dict_path, reverse=True, value_func=int)
        self.id2label_dict = load_kv_dict(label_dict_path)

    @property
    def vocab_size(self):
        """vocabulary size"""
        return max(self.word2id_dict.values()) + 1

    @property
    def num_labels(self):
        """num_labels"""
        return max(self.label2id_dict.values()) + 1

    def word_to_ids(self, words):
        """convert word to word index"""
        word_ids = []
        idx = 0
        try:
            words = unicode(words, 'utf-8')
        except:
            pass
        for word in words:
            if word not in self.word2id_dict:
                word = "OOV"
            word_id = self.word2id_dict[word]
            word_ids.append(word_id)
        return word_ids

    def label_to_ids(self, labels):
        """convert label to label index"""
        label_ids = []
        for label in labels:
            if label not in self.label2id_dict:
                label = "O"
            label_id = self.label2id_dict[label]
            label_ids.append(label_id)
        return label_ids

    def process(self, sent):
        words = sent.strip()
        word_ids = self.word_to_ids(words)
        return word_ids

    def parse_result(self, words, crf_decode):
        tags = [self.id2label_dict[str(x[0])] for x in crf_decode]

        sent_out = []
        tags_out = []
        partial_word = ""
        for ind, tag in enumerate(tags):
            if partial_word == "":
                partial_word = self.id2word_dict[str(words[ind])]
                tags_out.append(tag.split('-')[0])
                continue
            if tag.endswith("-B") or (tag == "O" and tag[ind - 1] != "O"):
                sent_out.append(partial_word)
                tags_out.append(tag.split('-')[0])
                partial_word = self.id2word_dict[str(words[ind])]
                continue
            partial_word += self.id2word_dict[str(words[ind])]

        if len(sent_out) < len(tags_out):
            sent_out.append(partial_word)

        return sent_out
