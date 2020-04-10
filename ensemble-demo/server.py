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
read_op = op_maker.create('general_reader')
g1_infer_op = op_maker.create('general_infer', node_name='g1')
g2_infer_op = op_maker.create('general_infer', node_name='g2')
add_op = op_maker.create('general_add')
response_op = op_maker.create('general_response')

op_seq_maker = OpSeqMaker()
op_seq_maker.add_op(read_op)
op_seq_maker.add_op(g1_infer_op, dependent_nodes=[read_op])
op_seq_maker.add_op(g2_infer_op, dependent_nodes=[read_op])
op_seq_maker.add_op(add_op, dependent_nodes=[g1_infer_op, g2_infer_op])
op_seq_maker.add_op(response_op, dependent_nodes=[add_op])

server = Server()
server.set_op_sequence(op_seq_maker.get_op_sequence())
# server.load_model_config(sys.argv[1])
model_configs = {'g1': 'imdb_bow_model', 'g2': 'imdb_cnn_model'}
server.load_model_config(model_configs)
server.prepare_server(workdir="work_dir1", port=9393, device="cpu")
server.run_server()
