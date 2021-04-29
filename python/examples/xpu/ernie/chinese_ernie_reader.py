#   Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
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
# coding=utf-8
from paddle_serving_app.reader.bert_base_reader import BertBaseReader
from paddle_serving_app.reader.batching import pad_batch_data
from paddle_serving_app.reader.tokenization import FullTokenizer, convert_to_unicode


class ChineseErnieReader(BertBaseReader):
    """
    ChineseErnieReader handles the most traditional Chinese Bert
    preprocessing, a user can define the vocab file through initialization

    Examples:
    from paddle_serving_app import ChineseErnieReader

    line = ["this is China"]
    reader = ChineseErnieReader()
    reader.process(line[0])

    """

    def __init__(self, args={}):
        super(ChineseErnieReader, self).__init__()
        vocab_file = ""
        if "vocab_file" in args:
            vocab_file = args["vocab_file"]
            print("vocab")
        else:
            vocab_file = self._download_or_not()

        self.tokenizer = FullTokenizer(vocab_file=vocab_file)
        print(self.tokenizer)
        if "max_seq_len" in args:
            self.max_seq_len = args["max_seq_len"]
        else:
            self.max_seq_len = 20
        self.vocab = self.tokenizer.vocab
        self.pad_id = self.vocab["[PAD]"]
        self.cls_id = self.vocab["[CLS]"]
        self.sep_id = self.vocab["[SEP]"]
        self.mask_id = self.vocab["[MASK]"]
        self.feed_keys = [
            "placeholder_0", "placeholder_1", "placeholder_2", "placeholder_3"
        ]

    """
    inner function
    """

    def _download_or_not(self):
        import os
        import paddle_serving_app
        module_path = os.path.dirname(paddle_serving_app.__file__)
        full_path = "{}/tmp/chinese_bert".format(module_path)
        os.system("mkdir -p {}".format(full_path))
        if os.path.exists("{}/vocab.txt".format(full_path)):
            pass
        else:
            url = "https://paddle-serving.bj.bcebos.com/reader/chinese_bert/vocab.txt"
            r = os.system("wget --no-check-certificate " + url)
            os.system("mv vocab.txt {}".format(full_path))
            if r != 0:
                raise SystemExit('Download failed, please check your network')
        return "{}/vocab.txt".format(full_path)

    """
    inner function
    """

    def _pad_batch(self, token_ids, text_type_ids, position_ids):
        batch_token_ids = [token_ids]
        batch_text_type_ids = [text_type_ids]
        batch_position_ids = [position_ids]

        padded_token_ids, input_mask = pad_batch_data(
            batch_token_ids,
            max_seq_len=self.max_seq_len,
            pad_idx=self.pad_id,
            return_input_mask=True)
        padded_text_type_ids = pad_batch_data(
            batch_text_type_ids,
            max_seq_len=self.max_seq_len,
            pad_idx=self.pad_id)
        padded_position_ids = pad_batch_data(
            batch_position_ids,
            max_seq_len=self.max_seq_len,
            pad_idx=self.pad_id)
        return padded_token_ids, padded_position_ids, padded_text_type_ids, input_mask

    """
    process function deals with a raw Chinese string as a sentence
    this funtion returns a feed_dict
    default key of the returned feed_dict: input_ids, position_ids, segment_ids, input_mask
    """

    def process(self, line):
        text_a = convert_to_unicode(line)
        tokens_a = self.tokenizer.tokenize(text_a)
        if len(tokens_a) > self.max_seq_len - 2:
            tokens_a = tokens_a[0:(self.max_seq_len - 2)]
        tokens = []
        text_type_ids = []
        tokens.append("[CLS]")
        text_type_ids.append(0)
        for token in tokens_a:
            tokens.append(token)
            text_type_ids.append(0)
        token_ids = self.tokenizer.convert_tokens_to_ids(tokens)
        position_ids = list(range(len(token_ids)))
        p_token_ids, p_pos_ids, p_text_type_ids, input_mask = \
            self._pad_batch(token_ids, text_type_ids, position_ids)
        feed_result = {
            self.feed_keys[0]: p_token_ids.reshape(-1).tolist(),
            self.feed_keys[1]: p_pos_ids.reshape(-1).tolist(),
            self.feed_keys[2]: p_text_type_ids.reshape(-1).tolist(),
            self.feed_keys[3]: input_mask.reshape(-1).tolist()
        }
        return feed_result
