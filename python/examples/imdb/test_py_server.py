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

from pyserver import Op
from pyserver import Channel
from pyserver import PyServer
import numpy as np
import python_service_channel_pb2
import logging

logging.basicConfig(
    format='%(asctime)s %(levelname)-8s [%(filename)s:%(lineno)d] %(message)s',
    datefmt='%Y-%m-%d %H:%M',
    level=logging.INFO)

# channel data: {name(str): data(bytes)}


class CombineOp(Op):
    def preprocess(self, input_data):
        cnt = 0
        for op_name, data in input_data.items():
            logging.debug("CombineOp preprocess: {}".format(op_name))
            cnt += np.frombuffer(data.insts[0].data, dtype='float')
        data = python_service_channel_pb2.ChannelData()
        inst = python_service_channel_pb2.Inst()
        inst.data = np.ndarray.tobytes(cnt)
        inst.name = "resp"
        data.insts.append(inst)
        return data

    def postprocess(self, output_data):
        return output_data


class UciOp(Op):
    def postprocess(self, output_data):
        data = python_service_channel_pb2.ChannelData()
        inst = python_service_channel_pb2.Inst()
        pred = np.array(output_data["price"][0][0], dtype='float')
        inst.data = np.ndarray.tobytes(pred)
        inst.name = "prediction"
        data.insts.append(inst)
        return data


read_channel = Channel(name="read_channel")
combine_channel = Channel(name="combine_channel")
out_channel = Channel(name="out_channel")

cnn_op = UciOp(
    name="cnn_op",
    input=read_channel,
    in_dtype='float',
    outputs=[combine_channel],
    out_dtype='float',
    server_model="./uci_housing_model",
    server_port="9393",
    device="cpu",
    client_config="uci_housing_client/serving_client_conf.prototxt",
    server_name="127.0.0.1:9393",
    fetch_names=["price"],
    concurrency=2)

bow_op = UciOp(
    name="bow_op",
    input=read_channel,
    in_dtype='float',
    outputs=[combine_channel],
    out_dtype='float',
    server_model="./uci_housing_model",
    server_port="9292",
    device="cpu",
    client_config="uci_housing_client/serving_client_conf.prototxt",
    server_name="127.0.0.1:9393",
    fetch_names=["price"],
    concurrency=2)

combine_op = CombineOp(
    name="combine_op",
    input=combine_channel,
    in_dtype='float',
    outputs=[out_channel],
    out_dtype='float',
    concurrency=2)

logging.info(read_channel.debug())
logging.info(combine_channel.debug())
logging.info(out_channel.debug())
pyserver = PyServer()
pyserver.add_channel(read_channel)
pyserver.add_channel(combine_channel)
pyserver.add_channel(out_channel)
pyserver.add_op(cnn_op)
pyserver.add_op(bow_op)
pyserver.add_op(combine_op)
pyserver.prepare_server(port=8080, worker_num=2)
pyserver.run_server()
