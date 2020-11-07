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
try:
    from paddle_serving_server_gpu.web_service import WebService, Op
except ImportError:
    from paddle_serving_server.web_service import WebService, Op
import logging
import numpy as np

_LOGGER = logging.getLogger()


class UciOp(Op):
    def init_op(self):
        self.separator = ","

    def preprocess(self, input_dicts):
        (_, input_dict), = input_dicts.items()
        _LOGGER.info(input_dict)
        x_value = input_dict["x"]
        if isinstance(x_value, (str, unicode)):
            input_dict["x"] = np.array(
                [float(x.strip())
                 for x in x_value.split(self.separator)]).reshape(1, 13)
        return input_dict

    def postprocess(self, input_dicts, fetch_dict):
        # _LOGGER.info(fetch_dict)
        fetch_dict["price"] = str(fetch_dict["price"][0][0])
        return fetch_dict


class UciService(WebService):
    def get_pipeline_response(self, read_op):
        uci_op = UciOp(name="uci", input_ops=[read_op])
        return uci_op


uci_service = UciService(name="uci")
uci_service.prepare_pipeline_config("config.yml")
uci_service.run_service()
