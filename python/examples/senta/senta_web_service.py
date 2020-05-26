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

from paddle_serving_server_gpu.web_service import WebService
from paddle_serving_client import Client
from paddle_serving_app.reader import LACReader, SentaReader
import os
import sys
from multiprocessing import Process


class SentaService(WebService):
    def set_config(
            self,
            lac_model_path,
            lac_dict_path,
            senta_dict_path, ):
        self.lac_model_path = lac_model_path
        self.lac_client_config_path = lac_model_path + "/serving_server_conf.prototxt"
        self.lac_dict_path = lac_dict_path
        self.senta_dict_path = senta_dict_path

    def start_lac_service(self):
        if not os.path.exists('./lac_serving'):
            os.mkdir("./lac_serving")
        os.chdir('./lac_serving')
        self.lac_port = self.port + 100
        r = os.popen(
            "python -m paddle_serving_server.serve --model {} --port {} &".
            format("../" + self.lac_model_path, self.lac_port))
        os.chdir('..')

    def init_lac_service(self):
        ps = Process(target=self.start_lac_service())
        ps.start()
        self.init_lac_client()

    def lac_predict(self, feed_data):
        lac_result = self.lac_client.predict(
            feed={"words": feed_data}, fetch=["crf_decode"])
        return lac_result

    def init_lac_client(self):
        self.lac_client = Client()
        self.lac_client.load_client_config(self.lac_client_config_path)
        self.lac_client.connect(["127.0.0.1:{}".format(self.lac_port)])

    def init_lac_reader(self):
        self.lac_reader = LACReader()

    def init_senta_reader(self):
        self.senta_reader = SentaReader()

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
senta_service.set_config(
    lac_model_path="./lac_model",
    lac_dict_path="./lac_dict",
    senta_dict_path="./vocab.txt")
senta_service.load_model_config(sys.argv[1])
senta_service.prepare_server(
    workdir=sys.argv[2], port=int(sys.argv[3]), device="cpu")
senta_service.init_lac_reader()
senta_service.init_senta_reader()
senta_service.init_lac_service()
senta_service.run_rpc_service()
senta_service.run_web_service()
