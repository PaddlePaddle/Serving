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

from paddle_serving_server.pipeline import Op, ReadOp
from paddle_serving_server.pipeline import PipelineServer
import numpy as np
import logging
from paddle_serving_app.reader import IMDBDataset

logging.basicConfig(
    format='%(asctime)s %(levelname)-8s [%(filename)s:%(lineno)d] %(message)s',
    datefmt='%Y-%m-%d %H:%M',
    # level=logging.DEBUG)
    level=logging.INFO)


class ImdbOp(Op):
    def preprocess(self, input_data):
        data = input_data.parse()
        imdb_dataset = IMDBDataset()
        imdb_dataset.load_resource('imdb.vocab')
        word_ids, _ = imdb_dataset.get_words_and_label(data['words'])
        return {"words": word_ids}

    # def postprocess(self, fetch_data):
    # return {key: str(value) for key, value in fetch_data.items()}


class CombineOp(Op):
    def preprocess(self, input_data):
        combined_prediction = 0
        for op_name, channeldata in input_data.items():
            data = channeldata.parse()
            logging.info("{}: {}".format(op_name, data["prediction"]))
            combined_prediction += data["prediction"]
        data = {"prediction": str(combined_prediction / 2)}
        return data


read_op = ReadOp()
bow_op = ImdbOp(
    name="bow",
    input_ops=[read_op],
    server_endpoints=["127.0.0.1:9393"],
    fetch_list=["prediction"],
    client_config="imdb_bow_client_conf/serving_client_conf.prototxt",
    concurrency=1,
    timeout=-1,
    retry=1)
cnn_op = ImdbOp(
    name="cnn",
    input_ops=[read_op],
    server_endpoints=["127.0.0.1:9292"],
    fetch_list=["prediction"],
    client_config="imdb_cnn_client_conf/serving_client_conf.prototxt",
    concurrency=1,
    timeout=-1,
    retry=1)
combine_op = CombineOp(
    name="combine",
    input_ops=[bow_op, cnn_op],
    concurrency=1,
    timeout=-1,
    retry=1)

server = PipelineServer()
server.add_ops([read_op, bow_op, cnn_op, combine_op])
# server.set_response_op(bow_op)
server.set_response_op(combine_op)
server.prepare_server('config.yml')
server.run_server()
