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

    executor = Executor(place=paddle.fluid.CPUPlace())
    paddle.fluid.io.save_persistables(executor, server_model_folder,
                                      main_program)
    

    

