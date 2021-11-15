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

_LOGGER = logging.getLogger()


class UciOp(Op):
    def init_op(self):
        self.separator = ","
        self.batch_separator = ";"

    def preprocess(self, input_dicts, data_id, log_id):
        (_, input_dict), = input_dicts.items()
        _LOGGER.error("UciOp::preprocess >>> log_id:{}, input:{}".format(
            log_id, input_dict))
        x_value = input_dict["x"].split(self.batch_separator)
        x_lst = []
        for x_val in x_value:
            x_lst.append(
                np.array([
                    float(x.strip()) for x in x_val.split(self.separator)
                ]).reshape(1, 13))
        input_dict["x"] = np.concatenate(x_lst, axis=0)
        proc_dict = {}
        return input_dict, False, None, ""

    def postprocess(self, input_dicts, fetch_dict, data_id, log_id):
        _LOGGER.info(
            "UciOp::postprocess >>> data_id:{}, log_id:{}, fetch_dict:{}".
            format(data_id, log_id, fetch_dict))
        fetch_dict["price"] = str(fetch_dict["price"])
        return fetch_dict, None, ""


class UciService(WebService):
    def get_pipeline_response(self, read_op):
        uci_op = UciOp(name="uci", input_ops=[read_op])
        return uci_op


uci_service = UciService(name="uci")
uci_service.prepare_pipeline_config("config.yml")
uci_service.run_service()
