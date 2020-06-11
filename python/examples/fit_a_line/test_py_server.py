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

from paddle_serving_server.pyserver import Op
from paddle_serving_server.pyserver import Channel
from paddle_serving_server.pyserver import PyServer
import numpy as np
import logging

logging.basicConfig(
    format='%(asctime)s %(levelname)-8s [%(filename)s:%(lineno)d] %(message)s',
    datefmt='%Y-%m-%d %H:%M',
    #level=logging.DEBUG)
    level=logging.INFO)


class CombineOp(Op):
    def preprocess(self, input_data):
        cnt = 0
        for op_name, channeldata in input_data.items():
            logging.debug("CombineOp preprocess: {}".format(op_name))
            data = channeldata.parse()
            cnt += data["price"]
        data = {"combine_op_output": cnt}
        return data


read_op = Op(name="read", inputs=None)
uci1_op = Op(name="uci1",
             inputs=[read_op],
             server_model="./uci_housing_model",
             server_port="9393",
             device="cpu",
             client_config="uci_housing_client/serving_client_conf.prototxt",
             server_name="127.0.0.1:9393",
             fetch_names=["price"],
             concurrency=1,
             timeout=0.1,
             retry=2)
uci2_op = Op(name="uci2",
             inputs=[read_op],
             server_model="./uci_housing_model",
             server_port="9292",
             device="cpu",
             client_config="uci_housing_client/serving_client_conf.prototxt",
             server_name="127.0.0.1:9292",
             fetch_names=["price"],
             concurrency=1,
             timeout=-1,
             retry=1)
combine_op = CombineOp(
    name="combine",
    inputs=[uci1_op, uci2_op],
    concurrency=1,
    timeout=-1,
    retry=1)

pyserver = PyServer(profile=False, retry=1)
pyserver.add_ops([read_op, uci1_op, uci2_op, combine_op])
pyserver.prepare_server(port=8080, worker_num=2)
pyserver.run_server()
