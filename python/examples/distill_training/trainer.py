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

import os
import sys
import time
import paddle.fluid as fluid
from nets import LACNet
from train_args import train_args

args = train_args()

net = LACNet()
net.set_args(args)
net.create_model()
words, target = net.get_inputs()
loss = net.get_loss()
#optimizer = fluid.optimizer.Adam(learning_rate=args.base_lr)
optimizer = fluid.optimizer.SGD(learning_rate=args.base_lr)
optimizer.minimize(loss)

dataset = fluid.DatasetFactory().create_dataset()
python_executable = "python"
pipe_command = "{} reader/seq_labeling_distill_reader.py {}".format(
    python_executable, args.endpoint_list)
thread_num = 8
batch_size = 4
dataset.set_thread(thread_num)
dataset.set_batch_size(batch_size)
dataset.set_use_var([words, target])
dataset.set_pipe_command(pipe_command)
whole_filelist = ["data/{}".format(x) for x in os.listdir("data")]
dataset.set_filelist(whole_filelist)

exe = fluid.Executor(fluid.CPUPlace())
exe.run(fluid.default_startup_program())
epochs = 10
for i in range(epochs):
    exe.train_from_dataset(
        program=fluid.default_main_program(), dataset=dataset, debug=True)
