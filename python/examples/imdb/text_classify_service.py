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

from paddle_serving_server.web_service import WebService
from paddle_serving_app.reader.imdb_reader import IMDBDataset
import sys
import numpy as np


class IMDBService(WebService):
    def prepare_dict(self, args={}):
        if len(args) == 0:
            exit(-1)
        self.dataset = IMDBDataset()
        self.dataset.load_resource(args["dict_file_path"])

    def preprocess(self, feed={}, fetch=[]):
        feed_batch = []
        words_lod = [0]
        is_batch = True
        for ins in feed:
            words = self.dataset.get_words_only(ins["words"])
            words = np.array(words).reshape(len(words), 1)
            words_lod.append(words_lod[-1] + len(words))
            feed_batch.append(words)
        feed = {"words": np.concatenate(feed_batch), "words.lod": words_lod}
        return feed, fetch, is_batch


imdb_service = IMDBService(name="imdb")
imdb_service.load_model_config(sys.argv[1])
imdb_service.prepare_server(
    workdir=sys.argv[2], port=int(sys.argv[3]), device="cpu")
imdb_service.prepare_dict({"dict_file_path": sys.argv[4]})
imdb_service.run_rpc_service()
imdb_service.run_web_service()
