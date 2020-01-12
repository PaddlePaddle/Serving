#   Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
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

from .serving_client import PredictorClient

class Client(object):
    def __init__(self):
        self.feed_names_ = []
        self.fetch_names_ = []
        self.client_handle_ = None
        self.feed_shapes_ = []
        self.feed_types_ = []
        self.feed_names_to_idx_ = {}

    def load_client_config(self, path):
        # load configuraion here
        # get feed vars, fetch vars
        # get feed shapes, feed types
        # map feed names to index
        return

    def connect(self, endpoint):
        # check whether current endpoint is available
        # init from client config
        # create predictor here
        return

    def get_feed_names(self):
        return self.feed_names_

    def get_fetch_names(self):
        return self.fetch_names_

    def predict(self, feed={}, fetch={}):
        int_slot = []
        float_slot = []
        int_feed_names = []
        float_feed_names = []
        fetch_names = []
        for key in feed:
            if key not in self.feed_names_:
                continue
            if self.feed_types_[key] == int_type:
                int_feed_names.append(key)
                int_slot.append(feed_map[key])
            elif self.feed_types_[key] == float_type:
                float_feed_names.append(key)
                float_slot.append(feed_map[key])

        for key in fetch:
            if key in self.fetch_names_:
                fetch_names.append(key)

        result = self.client_handle_.predict(
            float_slot, float_feed_names,
            int_slot, int_feed_names,
            fetch_names)
            
        result_map = {}
        for i, name in enumerate(fetch_names):
            result_map[name] = result[i]
            
        return result_map

