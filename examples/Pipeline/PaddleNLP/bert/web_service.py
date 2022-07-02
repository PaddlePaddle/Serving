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
from paddle_serving_server.web_service import WebService, Op
import logging
import numpy as np
import sys
from paddle_serving_app.reader import ChineseBertReader
_LOGGER = logging.getLogger()


class BertOp(Op):
    def init_op(self):
        self.reader = ChineseBertReader({
            "vocab_file": "vocab.txt",
            "max_seq_len": 128
        })

    def preprocess(self, input_dicts, data_id, log_id):
        (_, input_dict), = input_dicts.items()
        print("input dict", input_dict)
        batch_size = len(input_dict.keys())
        feed_res = []
        for i in range(batch_size):
            feed_dict = self.reader.process(input_dict[str(i)].encode("utf-8"))
            for key in feed_dict.keys():
                feed_dict[key] = np.array(feed_dict[key]).reshape(
                    (1, len(feed_dict[key]), 1))
            feed_res.append(feed_dict)
        feed_dict = {}
        for key in feed_res[0].keys():
            feed_dict[key] = np.concatenate([x[key] for x in feed_res], axis=0)
            print(key, feed_dict[key].shape)
        return feed_dict, False, None, ""

    def postprocess(self, input_dicts, fetch_dict, data_id, log_id):
        new_dict = {}
        new_dict["pooled_output"] = str(fetch_dict["pooled_output"])
        new_dict["sequence_output"] = str(fetch_dict["sequence_output"])
        return new_dict, None, ""


class BertService(WebService):
    def get_pipeline_response(self, read_op):
        bert_op = BertOp(name="bert", input_ops=[read_op])
        return bert_op


bert_service = BertService(name="bert")
bert_service.prepare_pipeline_config("config.yml")
bert_service.run_service()
