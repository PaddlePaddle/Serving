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
from .web_service import WebService
from flask import Flask, request


def parse_args():  # pylint: disable=doc-string-missing
    parser = argparse.ArgumentParser("serve")
    parser.add_argument(
        "--thread", type=int, default=10, help="Concurrency of server")
    parser.add_argument(
        "--model", type=str, default="", help="Model for serving")
    parser.add_argument(
        "--port", type=int, default=9292, help="Port the server")
    parser.add_argument(
        "--name", type=str, default="None", help="Web service name")
    parser.add_argument(
        "--workdir",
        type=str,
        default="workdir",
        help="Working dir of current service")
    parser.add_argument(
        "--device", type=str, default="cpu", help="Type of device")
    parser.add_argument(
        "--mem_optim_off",
        default=False,
        action="store_true",
        help="Memory optimize")
    parser.add_argument(
        "--ir_optim", default=False, action="store_true", help="Graph optimize")
    parser.add_argument(
        "--use_mkl", default=False, action="store_true", help="Use MKL")
    parser.add_argument(
        "--max_body_size",
        type=int,
        default=512 * 1024 * 1024,
        help="Limit sizes of messages")
    parser.add_argument(
        "--use_multilang",
        default=False,
        action="store_true",
        help="Use Multi-language-service")
    return parser.parse_args()


def start_standard_model():  # pylint: disable=doc-string-missing
    args = parse_args()
    thread_num = args.thread
    model = args.model
    port = args.port
    workdir = args.workdir
    device = args.device
    mem_optim = args.mem_optim_off is False
    ir_optim = args.ir_optim
    max_body_size = args.max_body_size
    use_mkl = args.use_mkl
    use_multilang = args.use_multilang

    if model == "":
        print("You must specify your serving model")
        exit(-1)

    import paddle_serving_server as serving
    op_maker = serving.OpMaker()
    read_op = op_maker.create('general_reader')
    general_infer_op = op_maker.create('general_infer')
    general_response_op = op_maker.create('general_response')

    op_seq_maker = serving.OpSeqMaker()
    op_seq_maker.add_op(read_op)
    op_seq_maker.add_op(general_infer_op)
    op_seq_maker.add_op(general_response_op)

    server = None
    if use_multilang:
        server = serving.MultiLangServer()
    else:
        server = serving.Server()
    server.set_op_sequence(op_seq_maker.get_op_sequence())
    server.set_num_threads(thread_num)
    server.set_memory_optimize(mem_optim)
    server.set_ir_optimize(ir_optim)
    server.use_mkl(use_mkl)
    server.set_max_body_size(max_body_size)
    server.set_port(port)

    server.load_model_config(model)
    server.prepare_server(workdir=workdir, port=port, device=device)
    server.run_server()


if __name__ == "__main__":

    args = parse_args()
    if args.name == "None":
        start_standard_model()
    else:
        service = WebService(name=args.name)
        service.load_model_config(args.model)
        service.prepare_server(
            workdir=args.workdir, port=args.port, device=args.device)
        service.run_rpc_service()

        app_instance = Flask(__name__)

        @app_instance.before_first_request
        def init():
            service._launch_web_service()

        service_name = "/" + service.name + "/prediction"

        @app_instance.route(service_name, methods=["POST"])
        def run():
            return service.get_prediction(request)

        app_instance.run(host="0.0.0.0",
                         port=service.port,
                         threaded=False,
                         processes=4)
