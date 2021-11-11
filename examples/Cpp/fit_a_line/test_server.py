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
import numpy as np


class UciService(WebService):
    def preprocess(self, feed=[], fetch=[]):
        feed_batch = []
        is_batch = True
        new_data = np.zeros((len(feed), 1, 13)).astype("float32")
        for i, ins in enumerate(feed):
            nums = np.array(ins["x"]).reshape(1, 1, 13)
            new_data[i] = nums
        feed = {"x": new_data}
        return feed, fetch, is_batch


uci_service = UciService(name="uci")
uci_service.load_model_config("uci_housing_model")
uci_service.prepare_server(workdir="workdir", port=9393)
uci_service.run_rpc_service()
uci_service.run_web_service()
