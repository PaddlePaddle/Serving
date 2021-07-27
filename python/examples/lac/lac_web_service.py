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
import sys
from paddle_serving_app.reader import LACReader
import numpy as np


class LACService(WebService):
    def load_reader(self):
        self.reader = LACReader()

    def preprocess(self, feed={}, fetch=[]):
        feed_batch = []
        fetch = ["crf_decode"]
        lod_info = [0]
        is_batch = True
        for ins in feed:
            if "words" not in ins:
                raise ("feed data error!")
            feed_data = self.reader.process(ins["words"])
            feed_batch.append(np.array(feed_data).reshape(len(feed_data), 1))
            lod_info.append(lod_info[-1] + len(feed_data))
        feed_dict = {
            "words": np.concatenate(
                feed_batch, axis=0),
            "words.lod": lod_info
        }
        return feed_dict, fetch, is_batch

    def postprocess(self, feed={}, fetch=[], fetch_map={}):
        batch_ret = []
        for idx, ins in enumerate(feed):
            begin = fetch_map['crf_decode.lod'][idx]
            end = fetch_map['crf_decode.lod'][idx + 1]
            segs = self.reader.parse_result(ins["words"],
                                            fetch_map["crf_decode"][begin:end])
            batch_ret.append({"word_seg": "|".join(segs)})
        return batch_ret


lac_service = LACService(name="lac")
lac_service.load_model_config(sys.argv[1])
lac_service.load_reader()
lac_service.prepare_server(
    workdir=sys.argv[2], port=int(sys.argv[3]), device="cpu")
lac_service.run_rpc_service()
lac_service.run_web_service()
