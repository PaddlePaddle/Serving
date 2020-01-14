#   Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
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


from paddle.fluid import Executor
from paddle.fluid.compiler import CompiledProgram
from paddle.fluid.framework import Program
from paddle.fluid.framework import default_main_program
from paddle.fluid import CPUPlace
from paddle.fluid.io import save_persistables
import os

def save_model(server_model_folder,
               client_config_folder,
               feed_var_dict,
               fetch_var_dict,
               main_program=None):
    if main_program is None:
        main_program = default_main_program()
    elif isinstance(main_program, CompiledProgram):
        main_program = main_program._program
        if main_program is None:
            raise TypeError("program should be as Program type or None")
    if not isinstance(main_program, Program):
        raise TypeError("program should be as Program type or None")

    executor = Executor(place=CPUPlace())

    save_persistables(executor, server_model_folder,
                      main_program)

    cmd = "mkdir -p {}".format(client_config_folder)
    os.system(cmd)
    with open("{}/client.conf".format(client_config_folder), "w") as fout:
        fout.write("{} {}\n".format(len(feed_var_dict), len(fetch_var_dict)))
        for key in feed_var_dict:
            fout.write("{}".format(key))
            if feed_var_dict[key].lod_level == 1:
                fout.write(" 1 -1\n")
            elif feed_var_dict[key].lod_level == 0:
                fout.write(" {}".format(len(feed_var_dict[key].shape)))
                for dim in feed_var_dict[key].shape:
                    fout.write(" {}".format(dim))
                fout.write("\n")
        for key in fetch_var_dict:
            fout.write("{} {}\n".format(key, fetch_var_dict[key].name))

    cmd = "cp {}/client.conf {}/server.conf".format(
        client_config_folder, server_model_folder)
    os.system(cmd)

    

