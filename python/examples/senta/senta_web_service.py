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
from paddle_serving_app import LACReader, SentaReader
import numpy as np
import os
import io
import sys
import subprocess
from multiprocessing import Process, Queue


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
        self.show = False

    def show_detail(self, show=False):
        self.show = show

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
        #self.init_lac_client()

    def lac_predict(self, feed_data):
        self.init_lac_client()
        lac_result = self.lac_client.predict(
            feed={"words": feed_data}, fetch=["crf_decode"])
        self.lac_client.release()
        return lac_result

    def init_lac_client(self):
        self.lac_client = Client()
        self.lac_client.load_client_config(self.lac_client_config_path)
        self.lac_client.connect(["127.0.0.1:{}".format(self.lac_port)])

    def init_lac_reader(self):
        self.lac_reader = LACReader(self.lac_dict_path)

    def init_senta_reader(self):
        self.senta_reader = SentaReader(vocab_path=self.senta_dict_path)

    def preprocess(self, feed=[], fetch=[]):
        feed_data = self.lac_reader.process(feed[0]["words"])
        if self.show:
            print("---- lac reader ----")
            print(feed_data)
        lac_result = self.lac_predict(feed_data)
        if self.show:
            print("---- lac out ----")
            print(lac_result)
        segs = self.lac_reader.parse_result(feed[0]["words"],
                                            lac_result["crf_decode"])
        if self.show:
            print("---- lac parse ----")
            print(segs)
        feed_data = self.senta_reader.process(segs)
        if self.show:
            print("---- senta reader ----")
            print("feed_data", feed_data)
        return {"words": feed_data}, fetch


senta_service = SentaService(name="senta")
#senta_service.show_detail(True)
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
senta_service.run_server()
senta_service.run_flask()
