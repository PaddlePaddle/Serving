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
import os
from multiprocessing import Pool, Process
from paddle_serving_server_gpu import serve_args
from flask import Flask, request


def start_gpu_card_model(index, gpuid, args):  # pylint: disable=doc-string-missing
    gpuid = int(gpuid)
    device = "gpu"
    port = args.port
    if gpuid == -1:
        device = "cpu"
    elif gpuid >= 0:
        port = args.port + index
    thread_num = args.thread
    model = args.model
    mem_optim = args.mem_optim_off is False
    ir_optim = args.ir_optim
    max_body_size = args.max_body_size
    use_multilang = args.use_multilang
    workdir = "{}_{}".format(args.workdir, gpuid)

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

    if use_multilang:
        server = serving.MultiLangServer()
    else:
        server = serving.Server()
    server.set_op_sequence(op_seq_maker.get_op_sequence())
    server.set_num_threads(thread_num)
    server.set_memory_optimize(mem_optim)
    server.set_ir_optimize(ir_optim)
    server.set_max_body_size(max_body_size)

    server.load_model_config(model)
    server.prepare_server(workdir=workdir, port=port, device=device)
    if gpuid >= 0:
        server.set_gpuid(gpuid)
    server.run_server()


def start_multi_card(args):  # pylint: disable=doc-string-missing
    gpus = ""
    if args.gpu_ids == "":
        gpus = []
    else:
        gpus = args.gpu_ids.split(",")
        if "CUDA_VISIBLE_DEVICES" in os.environ:
            env_gpus = os.environ["CUDA_VISIBLE_DEVICES"].split(",")
            for ids in gpus:
                if int(ids) >= len(env_gpus):
                    print(
                        " Max index of gpu_ids out of range, the number of CUDA_VISIBLE_DEVICES is {}.".
                        format(len(env_gpus)))
                    exit(-1)
        else:
            env_gpus = []
    if len(gpus) <= 0:
        print("gpu_ids not set, going to run cpu service.")
        start_gpu_card_model(-1, -1, args)
    else:
        gpu_processes = []
        for i, gpu_id in enumerate(gpus):
            p = Process(
                target=start_gpu_card_model, args=(
                    i,
                    gpu_id,
                    args, ))
            gpu_processes.append(p)
        for p in gpu_processes:
            p.start()
        for p in gpu_processes:
            p.join()


if __name__ == "__main__":
    args = serve_args()
    if args.name == "None":
        start_multi_card(args)
    else:
        from .web_service import WebService
        web_service = WebService(name=args.name)
        web_service.load_model_config(args.model)
        gpu_ids = args.gpu_ids
        if gpu_ids == "":
            if "CUDA_VISIBLE_DEVICES" in os.environ:
                gpu_ids = os.environ["CUDA_VISIBLE_DEVICES"]
        if len(gpu_ids) > 0:
            web_service.set_gpus(gpu_ids)
        web_service.prepare_server(
            workdir=args.workdir, port=args.port, device=args.device)
        web_service.run_rpc_service()

        app_instance = Flask(__name__)

        @app_instance.before_first_request
        def init():
            web_service._launch_web_service()

        service_name = "/" + web_service.name + "/prediction"

        @app_instance.route(service_name, methods=["POST"])
        def run():
            return web_service.get_prediction(request)

        app_instance.run(host="0.0.0.0",
                         port=web_service.port,
                         threaded=False,
                         processes=4)
