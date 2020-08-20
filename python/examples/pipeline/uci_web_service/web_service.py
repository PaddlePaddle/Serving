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

from paddle_serving_server_gpu.web_service import PipelineWebService
import logging
import numpy as np

_LOGGER = logging.getLogger()
user_handler = logging.StreamHandler()
user_handler.setLevel(logging.INFO)
user_handler.setFormatter(
    logging.Formatter(
        "%(levelname)s %(asctime)s [%(filename)s:%(lineno)d] %(message)s"))
_LOGGER.addHandler(user_handler)


class UciService(PipelineWebService):
    def init_separator(self):
        self.separator = ","

    def preprocess(self, input_dict):
        _LOGGER.info(input_dict)
        x_str = input_dict["x"]
        input_dict["x"] = np.array(
            [float(x.strip()) for x in x_str.split(self.separator)])
        return input_dict

    def postprocess(self, input_dict, fetch_dict):
        _LOGGER.info(fetch_dict)
        fetch_dict["price"] = str(fetch_dict["price"][0][0])
        return fetch_dict


uci_service = UciService(name="uci")
uci_service.init_separator()
uci_service.load_model_config("./uci_housing_model")
uci_service.set_gpus("0")
uci_service.prepare_server(workdir="workdir", port=18080, device="gpu")
uci_service.run_service()
