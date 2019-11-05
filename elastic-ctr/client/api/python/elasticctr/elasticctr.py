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

import elasticctr


class ElasticCTRPredictorApi(object):
    """
    A ElasticCTRPredictorApi Wrapper class
    """

    def __init__(self):
        self.api = elasticctr.ElasticCTRPredictorApi
        print "Init OK"

    def init(conf_dir, slot_conf_file, serving_conf_file):
        self.api.init(conf_dir, slot_conf_file, serving_conf_file)

    def thrd_initialize():
        self.api.thrd_initialize()

    def thrd_clear():
        self.api.thrd_clear()

    def destroy():
        self.api.destroy()

    def add_instance():
        return self.api.add_instance()

    def add_slot(instance, slot_name, value):
        return self.api.add_slot(instance, slot_name, value)

    def inference():
        self.api.inferene()

    def get_results():
        return self.api.get_results()
