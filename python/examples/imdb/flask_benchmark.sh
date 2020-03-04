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

# !/bin/bash

GLOG_v=2 $PYTHONROOT/bin/python test_server.py serving_server_model/ 9292 >serving.std 2>&1 &
$PYTHONROOT/bin/python imdb_flask.py serving_server_model/ > flask_webserve.log 2>&1 &

for i in {1,2,4,8,12,16,20,24}
do
    $PYTHONROOT/bin/python flask_client_multithread.py serving_client_conf/serving_client_conf.prototxt test_data/flask_data $i > flask_cli.profile 2>flask_cli.err 
    tail -1 flask.profile
    cat flask_cli.profile
done
