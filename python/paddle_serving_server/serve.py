# Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
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
"""
Usage:
    Host a trained paddle model with one line command
    Example:
        python -m paddle_serving_server.serve model 9292
"""

def start_standard_model(model_folder, port, thread_num):
    import paddle_serving_server as serving
    op_maker = serving.OpMaker()
    read_op = op_maker.create('general_reader')
    general_infer_op = op_maker.create('general_infer')
    general_response_op = op_maker.create('general_response')

    op_seq_maker = serving.OpSeqMaker()
    op_seq_maker.add_op(read_op)
    op_seq_maker.add_op(general_infer_op)
    op_seq_maker.add_op(general_response_op)

    server = Server()
    server.set_op_sequence(op_seq_maker.get_op_sequence())
    server.set_num_thread(thread_num)

    server.load_model_config(model_folder)
    port = port
    server.prepare_server(workdir="workdir", port=port, device="cpu")
    server.run_server()

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("{} model_folder port thread".format(sys.argv[0]))
        sys.exit(0)
    start_standard_model(sys.argv[1], int(sys.argv[2]), int(sys.argv[3]))
