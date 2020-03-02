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
#!flask/bin/python
from flask import Flask, request, abort
from paddle_serving_client import Client

class PluginService(object):
    def __init__(self, name="default_service", model="",
                 port=9091, concurrency=10):
        self.name = name
        self.port = port
        self.model = model
        self.concurrency = concurrency

    def prepare_service(self, args={}):
        return

    def start_service(self):
        app_instance = Flask(__name__)
        self.client_service = Client()
        self.client_service.load_client_config(
            "{}/serving_server_conf.prototxt".format(self.model))
        self.client_service.connect(["127.0.0.1:9292"])

        @app_instance.route('/PaddleServing/v1.0/prediction', methods=['POST'])
        def get_prediction():
            if not request.json:
                abort(400)
            if "fetch" not in request.json:
                abort(400)
            feed, fetch = self.preprocess(request.json, request.json["fetch"])
            fetch_map = self.client_service.predict(feed=feed, fetch=fetch)
            fetch_map = self.postprocess(feed=request.json, fetch=fetch, fetch_map=fetch_map)
        app_instance.run(host="127.0.0.1", port=self.port,
                         threaded=False, processes=1)

    def preprocess(self, feed={}, fetch=[]):
        return feed, fetch

    def postprocess(self, feed={}, fetch=[], fetch_map={}):
        return fetch_map
    
