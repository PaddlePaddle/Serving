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


# channel data: {name(str): data(bytes)}
class ImdbOp(Op):
    def preprocess(self, input_data):
        x = input_data[0]['words']
        feed = {"words": np.array(x)}
        return feed

    def postprocess(self, output_data):
        data = {"resp": fetch_map["prediction"][0][0]}
        return data


class CombineOp(Op):
    def preprocess(self, input_data):
        cnt = 0
        for data in input_data:
            cnt += data['resp']
        return {"resp": cnt}


read_channel = Channel(consumer=2)
cnn_out_channel = Channel()
bow_out_channel = Channel()
combine_out_channel = Channel()
cnn_op = ImdbOp(
    inputs=[read_channel],
    outputs=[cnn_out_channel],
    server_model="./imdb_cnn_model",
    server_port="9393",
    device="cpu",
    client_config="imdb_cnn_client_conf/serving_client_conf.prototxt",
    server_name="127.0.0.1:9393",
    fetch_names=["prediction"])
bow_op = ImdbOp(
    inputs=[read_channel],
    outputs=[bow_out_channel],
    server_model="./imdb_bow_model",
    server_port="9292",
    device="cpu",
    client_config="imdb_bow_client_conf/serving_client_conf.prototxt",
    server_name="127.0.0.1:9292",
    fetch_names=["prediction"])
combine_op = CombineOp(
    inputs=[cnn_out_channel, bow_out_channel], outputs=[combine_out_channel])

pyserver = PyServer()
pyserver.add_channel(read_channel)
pyserver.add_channel(cnn_out_channel)
pyserver.add_channel(bow_out_channel)
pyserver.add_channel(combine_out_channel)
pyserver.add_op(cnn_op)
pyserver.add_op(bow_op)
pyserver.add_op(combine_op)
pyserver.prepare_server(port=8080, worker_num=1)
pyserver.run_server()
