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
from paddle_serving_server_gpu.web_service import WebService
from bert_reader import BertReader
import sys
import os


class BertService(WebService):
    def load(self):
        self.reader = BertReader(vocab_file="vocab.txt", max_seq_len=128)

    def preprocess(self, feed={}, fetch=[]):
        feed_res = self.reader.process(feed["words"].encode("utf-8"))
        return feed_res, fetch


bert_service = BertService(name="bert")
bert_service.load()
bert_service.load_model_config(sys.argv[1])
gpu_ids = os.environ["CUDA_VISIBLE_DEVICES"]
bert_service.set_gpus(gpu_ids)
bert_service.prepare_server(
    workdir="workdir", port=int(sys.argv[2]), device="gpu")
bert_service.run_server()
