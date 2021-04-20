# coding=utf-8
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
from paddle_serving_server.web_service import WebService
from paddle_serving_app.reader import ChineseBertReader
import sys
import os
import numpy as np


class BertService(WebService):
    def load(self):
        self.reader = ChineseBertReader({
            "vocab_file": "vocab.txt",
            "max_seq_len": 128
        })

    def preprocess(self, feed=[], fetch=[]):
        feed_res = []
        is_batch = False
        for ins in feed:
            feed_dict = self.reader.process(ins["words"].encode("utf-8"))
            for key in feed_dict.keys():
                feed_dict[key] = np.array(feed_dict[key]).reshape(
                    (len(feed_dict[key]), 1))
            feed_res.append(feed_dict)
        return feed_res, fetch, is_batch


bert_service = BertService(name="bert")
bert_service.load()
bert_service.load_model_config(sys.argv[1])
bert_service.prepare_server(
    workdir="workdir", port=int(sys.argv[2]), device="gpu")
bert_service.run_rpc_service()
bert_service.run_web_service()
