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

from paddle_serving_client import Client
import sys
import os
import time
from paddle_serving_app.pddet import Detection, ArgParse
import numpy as np

py_version = sys.version_info[0]

feed_var_names = ['image', 'im_shape', 'im_info']
fetch_var_names = ['multiclass_nms']
FLAGS = ArgParse()
pddet = Detection(FLAGS.config_path, FLAGS.visualize, FLAGS.dump_result,
                  FLAGS.output_dir)
feed_dict = pddet.preprocess(feed_var_names, FLAGS.infer_img)
client = Client()
client.load_client_config(FLAGS.serving_client_conf)
client.connect(['127.0.0.1:9494'])
fetch_map = client.predict(feed=feed_dict, fetch=fetch_var_names)
outs = fetch_map.values()
pddet.postprocess(fetch_map, fetch_var_names)
