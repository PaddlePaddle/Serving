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

import paddlehub as hub
import paddle.fluid as fluid
import paddle_serving_client.io as serving_io

model_name = "bert_chinese_L-12_H-768_A-12"
module = hub.Module(model_name)
inputs, outputs, program = module.context(trainable=True, max_seq_len=20)
place = fluid.core_avx.CPUPlace()
exe = fluid.Executor(place)
input_ids = inputs["input_ids"]
position_ids = inputs["position_ids"]
segment_ids = inputs["segment_ids"]
input_mask = inputs["input_mask"]
pooled_output = outputs["pooled_output"]
sequence_output = outputs["sequence_output"]

feed_var_names = [
    input_ids.name, position_ids.name, segment_ids.name, input_mask.name
]

target_vars = [pooled_output, sequence_output]

serving_io.save_model("serving_server_model", "serving_client_conf", {
    "input_ids": input_ids,
    "position_ids": position_ids,
    "segment_ids": segment_ids,
    "input_mask": input_mask,
}, {"pooled_output": pooled_output,
    "sequence_output": sequence_output}, program)
