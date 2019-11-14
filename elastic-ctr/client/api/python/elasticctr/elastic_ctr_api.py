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
import httplib
import sys
import os


class ElasticCTRAPI(object):
    def __init__(self, serving_ip, serving_port):
        self._instances = []
        self._slots = []
        self._conn = self.conn(serving_ip, serving_port)

    def read_slots_conf(self, slots_conf_file):
        if not os.path.exists(slots_conf_file):
            print "Path %s not exist" % sltos_conf_file
            return -1
        with open(slots_conf_file, "r") as f:
            for line in f:
                self._slots.append(line.rstrip('\n'))
        return 0

    def conn(self, ip, port):
        return httplib.HTTPConnection(ip, port)

    def add_instance(self):
        feature_slots = []
        instance = [{"slots": feature_slots}]
        self._instances += instance
        return instance

    def add_slot(self, instance, slot, feasigns):
        if not isinstance(instance, list):
            print "add slot: parameter invalid: instance should be list"
            return -1

        if not isinstance(feasigns, list):
            print "add slot: value format invalid: feasigns should be list"
            return -1

        kv = [{"slot_name": slot, "feasigns": feasigns}]
        instance[0]["slots"] += kv

    def inference(self):
        for instance in self._instances:
            feature_slots = instance["slots"]
            keys = []
            for x in feature_slots:
                keys += [x["slot_name"]]
            for slot in self._slots:
                if not slot in keys:
                    feature_slots += [{"slot_name": slot, "feasigns": [0]}]
        req = {"instances": self._instances}

        request_json = json.dumps(req)

        try:
            self._conn.request('POST', "/ElasticCTRPredictionService/inference",
                               request_json,
                               {"Content-Type": "application/json"})
            response = self._conn.getresponse()
            return response.read()
        except httplib.HTTPException as e:
            print e.reason
