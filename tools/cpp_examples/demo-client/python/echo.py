#   Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
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

import json
import urllib2

data = {"a": 1, "b": 0.5}
request_json = json.dumps(data)

req = urllib2.Request("http://127.0.0.1:8010/BuiltinTestEchoService/inference")
try:
    response = urllib2.urlopen(req, request_json, 1)
    print response.read()
except urllib2.HTTPError as e:
    print e.reason
