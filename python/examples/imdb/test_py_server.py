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
"""
class ImdbOp(Op):
    def postprocess(self, output_data):
        data = python_service_channel_pb2.ChannelData()
        inst = python_service_channel_pb2.Inst()
        pred = np.array(output_data["prediction"][0][0], dtype='float')
        inst.data = np.ndarray.tobytes(pred)
        inst.name = "prediction"
        inst.id = 0 #TODO
        data.insts.append(inst)
        return data
"""


class CombineOp(Op):
    def preprocess(self, input_data):
        data_id = None
        cnt = 0
        for input in input_data:
            data = input[0]  # batchsize=1
            cnt += np.frombuffer(data.insts[0].data, dtype='float')
            if data_id is None:
                data_id = data.id
            if data_id != data.id:
                raise Exception("id not match: {} vs {}".format(data_id,
                                                                data.id))
        data = python_service_channel_pb2.ChannelData()
        inst = python_service_channel_pb2.Inst()
        inst.data = np.ndarray.tobytes(cnt)
        inst.name = "resp"
        data.insts.append(inst)
        data.id = data_id
        print(data)
        return data


class UciOp(Op):
    def postprocess(self, output_data):
        data_ids = self.get_data_ids()
        data = python_service_channel_pb2.ChannelData()
        inst = python_service_channel_pb2.Inst()
        pred = np.array(output_data["price"][0][0], dtype='float')
        inst.data = np.ndarray.tobytes(pred)
        inst.name = "prediction"
        data.insts.append(inst)
        data.id = data_ids[0]
        return data


read_channel = Channel(consumer=2)
cnn_out_channel = Channel()
bow_out_channel = Channel()
combine_out_channel = Channel()
cnn_op = UciOp(
    inputs=[read_channel],
    in_dtype='float',
    outputs=[cnn_out_channel],
    out_dtype='float',
    server_model="./uci_housing_model",
    server_port="9393",
    device="cpu",
    client_config="uci_housing_client/serving_client_conf.prototxt",
    server_name="127.0.0.1:9393",
    fetch_names=["price"])
bow_op = UciOp(
    inputs=[read_channel],
    in_dtype='float',
    outputs=[bow_out_channel],
    out_dtype='float',
    server_model="./uci_housing_model",
    server_port="9292",
    device="cpu",
    client_config="uci_housing_client/serving_client_conf.prototxt",
    server_name="127.0.0.1:9393",
    fetch_names=["price"])
'''
cnn_op = ImdbOp(
    inputs=[read_channel],
    outputs=[cnn_out_channel],
    server_model="./imdb_cnn_model",
    server_port="9393",
    device="cpu",
    client_config="imdb_cnn_client_conf/serving_client_conf.prototxt",
    server_name="127.0.0.1:9393",
    fetch_names=["acc", "cost", "prediction"])
bow_op = ImdbOp(
    inputs=[read_channel],
    outputs=[bow_out_channel],
    server_model="./imdb_bow_model",
    server_port="9292",
    device="cpu",
    client_config="imdb_bow_client_conf/serving_client_conf.prototxt",
    server_name="127.0.0.1:9292",
    fetch_names=["acc", "cost", "prediction"])
'''
combine_op = CombineOp(
    inputs=[cnn_out_channel, bow_out_channel],
    in_dtype='float',
    outputs=[combine_out_channel],
    out_dtype='float')

pyserver = PyServer()
pyserver.add_channel(read_channel)
pyserver.add_channel(cnn_out_channel)
pyserver.add_channel(bow_out_channel)
pyserver.add_channel(combine_out_channel)
pyserver.add_op(cnn_op)
pyserver.add_op(bow_op)
pyserver.add_op(combine_op)
pyserver.prepare_server(port=8080, worker_num=2)
pyserver.run_server()
