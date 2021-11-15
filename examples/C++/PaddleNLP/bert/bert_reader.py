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
# pylint: disable=doc-string-missing
from batching import pad_batch_data
import tokenization


class BertReader():
    def __init__(self, vocab_file="", max_seq_len=128):
        self.vocab_file = vocab_file
        self.tokenizer = tokenization.FullTokenizer(vocab_file=vocab_file)
        self.max_seq_len = max_seq_len
        self.vocab = self.tokenizer.vocab
        self.pad_id = self.vocab["[PAD]"]
        self.cls_id = self.vocab["[CLS]"]
        self.sep_id = self.vocab["[SEP]"]
        self.mask_id = self.vocab["[MASK]"]

    def pad_batch(self, token_ids, text_type_ids, position_ids):
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

    def process(self, sent):
        text_a = tokenization.convert_to_unicode(sent)
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
            self.pad_batch(token_ids, text_type_ids, position_ids)
        feed_result = {
            "input_ids": p_token_ids.reshape(-1).tolist(),
            "position_ids": p_pos_ids.reshape(-1).tolist(),
            "segment_ids": p_text_type_ids.reshape(-1).tolist(),
            "input_mask": input_mask.reshape(-1).tolist()
        }
        return feed_result
