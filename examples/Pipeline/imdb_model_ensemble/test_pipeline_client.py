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
from paddle_serving_server.pipeline import PipelineClient
import numpy as np

client = PipelineClient()
client.connect(['127.0.0.1:18070'])

words = 'i am very sad | 0'

futures = []
for i in range(100):
    futures.append(
        client.predict(
            feed_dict={"words": words,
                       "logid": 10000 + i},
            fetch=["prediction"],
            asyn=True,
            profile=False))

for f in futures:
    res = f.result()
    if res.err_no != 0:
        print("predict failed: {}".format(res))
    print(res)
