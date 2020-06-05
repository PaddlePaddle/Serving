#encoding=utf-8
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

from paddle_serving_server.web_service import WebService
from paddle_serving_client import Client
from paddle_serving_app.reader import LACReader, SentaReader
import os
import sys

#senta_web_service.py
from paddle_serving_server.web_service import WebService
from paddle_serving_client import Client
from paddle_serving_app.reader import LACReader, SentaReader


class SentaService(WebService):
    #初始化lac模型预测服务
    def init_lac_client(self, lac_port, lac_client_config):
        self.lac_reader = LACReader()
        self.senta_reader = SentaReader()
        self.lac_client = Client()
        self.lac_client.load_client_config(lac_client_config)
        self.lac_client.connect(["127.0.0.1:{}".format(lac_port)])

    #定义senta模型预测服务的预处理，调用顺序：lac reader->lac模型预测->预测结果后处理->senta reader
    def preprocess(self, feed=[], fetch=[]):
        feed_data = [{
            "words": self.lac_reader.process(x["words"])
        } for x in feed]
        lac_result = self.lac_client.predict(
            feed=feed_data, fetch=["crf_decode"])
        feed_batch = []
        result_lod = lac_result["crf_decode.lod"]
        for i in range(len(feed)):
            segs = self.lac_reader.parse_result(
                feed[i]["words"],
                lac_result["crf_decode"][result_lod[i]:result_lod[i + 1]])
            feed_data = self.senta_reader.process(segs)
            feed_batch.append({"words": feed_data})
        return feed_batch, fetch


senta_service = SentaService(name="senta")
senta_service.load_model_config("senta_bilstm_model")
senta_service.prepare_server(workdir="workdir")
senta_service.init_lac_client(
    lac_port=9300, lac_client_config="lac_model/serving_server_conf.prototxt")
senta_service.run_rpc_service()
senta_service.run_web_service()
