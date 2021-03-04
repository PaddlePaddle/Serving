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
import json
import base64
import time
from multiprocessing import Pool, Process
from paddle_serving_server_gpu import serve_args
from flask import Flask, request
import sys
if sys.version_info.major == 2:
    from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
elif sys.version_info.major == 3:
    from http.server import BaseHTTPRequestHandler, HTTPServer


def start_gpu_card_model(index, gpuid, port, args):  # pylint: disable=doc-string-missing
    gpuid = int(gpuid)
    device = "gpu"
    if gpuid == -1:
        device = "cpu"
    elif gpuid >= 0:
        port = port + index
    thread_num = args.thread
    model = args.model
    mem_optim = args.mem_optim_off is False
    ir_optim = args.ir_optim
    max_body_size = args.max_body_size
    use_multilang = args.use_multilang
    workdir = args.workdir
    if gpuid >= 0:
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
    if args.use_trt:
        server.set_trt()

    if args.use_lite:
        server.set_lite()
        device = "arm"

    server.set_device(device)
    if args.use_xpu:
        server.set_xpu()

    if args.product_name != None:
        server.set_product_name(args.product_name)
    if args.container_id != None:
        server.set_container_id(args.container_id)

    server.load_model_config(model)
    server.prepare_server(
        workdir=workdir,
        port=port,
        device=device,
        use_encryption_model=args.use_encryption_model)
    if gpuid >= 0:
        server.set_gpuid(gpuid)
    server.run_server()


def start_multi_card(args, serving_port=None):  # pylint: disable=doc-string-missing
    gpus = ""
    if serving_port == None:
        serving_port = args.port
    if args.gpu_ids == "":
        gpus = []
    else:
        gpus = args.gpu_ids.split(",")
        if "CUDA_VISIBLE_DEVICES" in os.environ:
            env_gpus = os.environ["CUDA_VISIBLE_DEVICES"].split(",")
            for ids in gpus:
                if int(ids) >= len(env_gpus):
                    print(
                        " Max index of gpu_ids out of range, the number of CUDA_VISIBLE_DEVICES is {}."
                        .format(len(env_gpus)))
                    exit(-1)
        else:
            env_gpus = []
    if args.use_lite:
        print("run arm server.")
        start_gpu_card_model(-1, -1, args)
    elif len(gpus) <= 0:
        print("gpu_ids not set, going to run cpu service.")
        start_gpu_card_model(-1, -1, serving_port, args)
    else:
        gpu_processes = []
        for i, gpu_id in enumerate(gpus):
            p = Process(
                target=start_gpu_card_model,
                args=(
                    i,
                    gpu_id,
                    serving_port,
                    args, ))
            gpu_processes.append(p)
        for p in gpu_processes:
            p.start()
        for p in gpu_processes:
            p.join()


class MainService(BaseHTTPRequestHandler):
    def get_available_port(self):
        default_port = 12000
        for i in range(1000):
            if port_is_available(default_port + i):
                return default_port + i

    def start_serving(self):
        start_multi_card(args, serving_port)

    def get_key(self, post_data):
        if "key" not in post_data:
            return False
        else:
            key = base64.b64decode(post_data["key"].encode())
            with open(args.model + "/key", "wb") as f:
                f.write(key)
            return True

    def check_key(self, post_data):
        if "key" not in post_data:
            return False
        else:
            key = base64.b64decode(post_data["key"].encode())
            with open(args.model + "/key", "rb") as f:
                cur_key = f.read()
            return (key == cur_key)

    def start(self, post_data):
        post_data = json.loads(post_data)
        global p_flag
        if not p_flag:
            if args.use_encryption_model:
                print("waiting key for model")
                if not self.get_key(post_data):
                    print("not found key in request")
                    return False
            global serving_port
            global p
            serving_port = self.get_available_port()
            p = Process(target=self.start_serving)
            p.start()
            time.sleep(3)
            if p.is_alive():
                p_flag = True
            else:
                return False
        else:
            if p.is_alive():
                if not self.check_key(post_data):
                    return False
            else:
                return False
        return True

    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)
        if self.start(post_data):
            response = {"endpoint_list": [serving_port]}
        else:
            response = {"message": "start serving failed"}
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps(response).encode())


if __name__ == "__main__":
    args = serve_args()
    if args.name == "None":
        from .web_service import port_is_available
        if args.use_encryption_model:
            p_flag = False
            p = None
            serving_port = 0
            server = HTTPServer(('localhost', int(args.port)), MainService)
            print(
                'Starting encryption server, waiting for key from client, use <Ctrl-C> to stop'
            )
            server.serve_forever()
        else:
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
            workdir=args.workdir,
            port=args.port,
            device=args.device,
            use_lite=args.use_lite,
            use_xpu=args.use_xpu,
            ir_optim=args.ir_optim)
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
