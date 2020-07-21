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

from paddle_serving_server import OpMaker
from paddle_serving_server import OpGraphMaker
from paddle_serving_server import MultiLangServer

op_maker = OpMaker()
read_op = op_maker.create('general_reader')
cnn_infer_op = op_maker.create(
    'general_infer', engine_name='cnn', inputs=[read_op])
bow_infer_op = op_maker.create(
    'general_infer', engine_name='bow', inputs=[read_op])
response_op = op_maker.create(
    'general_response', inputs=[cnn_infer_op, bow_infer_op])

op_graph_maker = OpGraphMaker()
op_graph_maker.add_op(read_op)
op_graph_maker.add_op(cnn_infer_op)
op_graph_maker.add_op(bow_infer_op)
op_graph_maker.add_op(response_op)

server = MultiLangServer()
server.set_op_graph(op_graph_maker.get_op_graph())
model_config = {cnn_infer_op: 'imdb_cnn_model', bow_infer_op: 'imdb_bow_model'}
server.load_model_config(model_config)
server.prepare_server(workdir="work_dir1", port=9393, device="cpu")
server.run_server()
