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
"""
Usage:
    Host a trained paddle model with one line command
    Example:
        python -m paddle_serving_server.serve --model ./serving_server_model --port 9292
"""
import argparse
from multiprocessing import Pool, Process


def parse_args():
    parser = argparse.ArgumentParser("serve")
    parser.add_argument(
        "--thread", type=int, default=10, help="Concurrency of server")
    parser.add_argument(
        "--model", type=str, default="", help="Model for serving")
    parser.add_argument(
        "--port", type=int, default=9292, help="Port of the starting gpu")
    parser.add_argument(
        "--workdir",
        type=str,
        default="workdir",
        help="Working dir of current service")
    parser.add_argument(
        "--device", type=str, default="gpu", help="Type of device")
    parser.add_argument(
        "--gpu_ids", type=int, default=0, help="gpu ids")
    return parser.parse_args()

args = parse_args()

def start_gpu_card_model(gpuid):
    device = "gpu"
    port = args.port
    if gpuid == -1:
        device = "cpu"
        port = args.port + gpuid
    thread_num = args.thread
    model = args.model
    workdir = args.workdir

    if model == "":
        print("You must specify your serving model")
        exit(-1)

    import paddle_serving_server_gpu as serving
    op_maker = serving.OpMaker()
    read_op = op_maker.create('general_reader')
    general_infer_op = op_maker.create('general_infer')
    general_response_op = op_maker.create('general_response')
    
    op_seq_maker = serving.OpSeqMaker()
    op_seq_maker.add_op(read_op)
    op_seq_maker.add_op(general_infer_op)
    op_seq_maker.add_op(general_response_op)

    server = serving.Server()
    server.set_op_sequence(op_seq_maker.get_op_sequence())
    server.set_num_threads(thread_num)

    server.load_model_config(model)
    server.prepare_server(workdir=workdir, port=port, device=device)
    if gpuid >= 0:
        server.set_gpuid(gpuid)
    server.run_server()

if __name__ == "__main__":
    gpus = args.gpu_ids.split(",")
    if len(gpus) <= 0:
        start_gpu_card_model(-1)
    else:
        gpu_processes = []
        for i, gpu_id in gpus:
            p = Process(target=start_gpu_card_model, (i,))
            gpu_processes.append(p)
        for p in gpu_processes:
            p.start()
        for p in gpu_processes:
            p.join()
    
