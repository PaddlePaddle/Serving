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
from paddle_serving_server_gpu import pipeline
import numpy as np
import logging

_LOGGER = logging.getLogger()


class UciOp(pipeline.Op):
    def init_op(self):
        self.separator = ","

    def preprocess(self, input_dicts):
        (_, input_dict), = input_dicts.items()
        _LOGGER.info(input_dict)
        x_value = input_dict["x"]
        if isinstance(x_value, (str, unicode)):
            input_dict["x"] = np.array(
                [float(x.strip()) for x in x_value.split(self.separator)])
        return input_dict

    def postprocess(self, input_dicts, fetch_dict):
        _LOGGER.info(fetch_dict)
        fetch_dict["price"] = str(fetch_dict["price"][0][0])
        return fetch_dict


read_op = pipeline.RequestOp()
uci_op = UciOp(
    name="uci",
    input_ops=[read_op],
    local_rpc_service_handler=pipeline.LocalRpcServiceHandler(
        model_config="uci_housing_model",
        workdir="workdir",
        thread_num=2,
        devices="0",  # if devices="", use cpu
        mem_optim=True,
        ir_optim=False),
    concurrency=1)
response_op = pipeline.ResponseOp(input_ops=[uci_op])

server = pipeline.PipelineServer()
server.set_response_op(response_op)
server.prepare_server('config.yml')
server.start_local_rpc_service()  # after prepare_server
server.run_server()
