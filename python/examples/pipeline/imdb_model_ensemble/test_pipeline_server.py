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

from paddle_serving_server.pipeline import Op
from paddle_serving_server.pipeline import PipelineServer
import numpy as np
import logging

logging.basicConfig(
    format='%(asctime)s %(levelname)-8s [%(filename)s:%(lineno)d] %(message)s',
    datefmt='%Y-%m-%d %H:%M',
    #level=logging.DEBUG)
    level=logging.INFO)


class CombineOp(Op):
    def preprocess(self, input_data):
        combined_prediction = 0
        for op_name, channeldata in input_data.items():
            data = channeldata.parse()
            logging.info("{}: {}".format(op_name, data["prediction"]))
            combined_prediction += data["prediction"]
        data = {"prediction": combined_prediction / 2}
        return data


read_op = Op(name="read", inputs=None)
bow_op = Op(name="bow",
            inputs=[read_op],
            server_model="imdb_bow_model",
            server_port="9393",
            device="cpu",
            client_config="imdb_bow_client_conf/serving_client_conf.prototxt",
            server_name="127.0.0.1:9393",
            fetch_names=["prediction"],
            concurrency=1,
            timeout=0.1,
            retry=2)
cnn_op = Op(name="cnn",
            inputs=[read_op],
            server_model="imdb_cnn_model",
            server_port="9292",
            device="cpu",
            client_config="imdb_cnn_client_conf/serving_client_conf.prototxt",
            server_name="127.0.0.1:9292",
            fetch_names=["prediction"],
            concurrency=1,
            timeout=-1,
            retry=1)
combine_op = CombineOp(
    name="combine", inputs=[bow_op, cnn_op], concurrency=1, timeout=-1, retry=1)

pyserver = PipelineServer(
    use_multithread=True,
    client_type='grpc',
    use_future=False,
    profile=False,
    retry=1)
pyserver.add_ops([read_op, bow_op, cnn_op, combine_op])
pyserver.prepare_server(port=8080, worker_num=2)
pyserver.run_server()
