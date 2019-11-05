# Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
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

import os
import os.path
import sys

path = os.path.abspath(os.path.curdir) + '/../api'
print path
sys.path.append(path)

import lib
from lib import elasticctr

api = elasticctr.ElasticCTRPredictorApi()
ret = api.init("./conf", "slot.conf", "predictors.prototxt")

if ret != 0:
    print "api.init fail"
    sys.exit(-1)

api.thrd_initialize()

api.thrd_clear()

instance = api.add_instance()
api.add_slot(instance, "0", 1234)
api.add_slot(instance, "1", 1234)
api.add_slot(instance, "2", 1234)
api.add_slot(instance, "3", 1234)
api.add_slot(instance, "4", 1234)
api.add_slot(instance, "5", 1234)
api.add_slot(instance, "6", 1234)

ret = api.inference()

print ret

api.destroy()
