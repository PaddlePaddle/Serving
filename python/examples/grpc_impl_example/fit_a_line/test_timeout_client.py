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

from paddle_serving_client import MultiLangClient as Client
import grpc
import numpy as np
client = Client()
client.connect(["127.0.0.1:9393"])
client.set_rpc_timeout_ms(40)

x = [
    0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283,
    0.4919, 0.1856, 0.0795, -0.0332
]
for i in range(3):
    new_data = np.array(x).astype("float32").reshape((1, 13))
    fetch_map = client.predict(
        feed={"x": new_data}, fetch=["price"], batch=False)
    if fetch_map["serving_status_code"] == 0:
        print(fetch_map)
    elif fetch_map["serving_status_code"] == grpc.StatusCode.DEADLINE_EXCEEDED:
        print('timeout')
    else:
        print(fetch_map["serving_status_code"])
