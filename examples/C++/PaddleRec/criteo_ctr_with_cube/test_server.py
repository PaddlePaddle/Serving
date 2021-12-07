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
# pylint: disable=doc-string-missing

import os
import sys
from paddle_serving_server import OpMaker
from paddle_serving_server import OpSeqMaker
from paddle_serving_server import Server

op_maker = OpMaker()
read_op = op_maker.create('GeneralReaderOp')
general_dist_kv_infer_op = op_maker.create('GeneralDistKVInferOp')
response_op = op_maker.create('GeneralResponseOp')

op_seq_maker = OpSeqMaker()
op_seq_maker.add_op(read_op)
op_seq_maker.add_op(general_dist_kv_infer_op)
op_seq_maker.add_op(response_op)

server = Server()
server.set_op_sequence(op_seq_maker.get_op_sequence())
server.set_num_threads(4)
server.load_model_config(sys.argv[1])
server.prepare_server(
    workdir="work_dir1",
    port=9292,
    device="cpu",
    cube_conf="./cube/conf/cube.conf")
server.run_server()
