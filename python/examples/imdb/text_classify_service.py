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
from plugin_service import PluginService
import sys

class IMDBService(PluginService):
    def prepare_service(self, args={}):
        if len(args) == 0:
            exit(-1)
        self.word_dict = {}
        with open(args["dict_file_path"]) as fin:
            idx = 0
            for line in fin:
                self.word_dict[idx] = idx
                idx += 1
    
    def preprocess(self, feed={}, fetch=[]):
        if "words" not in feed:
            exit(-1)
        res_feed = {}
        res_feed["words"] = [self.word_dict[int(x)] for x in feed["words"]]
        print(res_feed)
        return res_feed, fetch

imdb_service = IMDBService(name="imdb", model=sys.argv[1], port=9898)
imdb_service.prepare_service({"dict_file_path":sys.argv[2]})
imdb_service.start_service()
