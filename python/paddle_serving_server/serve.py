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
from multiprocessing import Process
import sys
if sys.version_info.major == 2:
    from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
elif sys.version_info.major == 3:
    from http.server import BaseHTTPRequestHandler, HTTPServer

from contextlib import closing
import socket
from paddle_serving_server.env import CONF_HOME
import signal
from paddle_serving_server.util import *
from paddle_serving_server.env_check.run import check_env
import cmd


def signal_handler(signal, frame):
    print('Process stopped')
    sys.exit(0)


signal.signal(signal.SIGINT, signal_handler)


# web_service.py is still used by Pipeline.
def port_is_available(port):
    with closing(socket.socket(socket.AF_INET, socket.SOCK_STREAM)) as sock:
        sock.settimeout(2)
        result = sock.connect_ex(('127.0.0.1', port))
    if result != 0:
        return True
    else:
        return False


def format_gpu_to_strlist(unformatted_gpus):
    gpus_strlist = []
    if isinstance(unformatted_gpus, int):
        gpus_strlist = [str(unformatted_gpus)]
    elif isinstance(unformatted_gpus, list):
        if unformatted_gpus == [""]:
            gpus_strlist = ["-1"]
        elif len(unformatted_gpus) == 0:
            gpus_strlist = ["-1"]
        else:
            gpus_strlist = [str(x) for x in unformatted_gpus]
    elif isinstance(unformatted_gpus, str):
        if unformatted_gpus == "":
            gpus_strlist = ["-1"]
        else:
            gpus_strlist = [unformatted_gpus]
    elif unformatted_gpus == None:
        gpus_strlist = ["-1"]
    else:
        raise ValueError("error input of set_gpus")

    # check cuda visible
    if "CUDA_VISIBLE_DEVICES" in os.environ:
        env_gpus = os.environ["CUDA_VISIBLE_DEVICES"].split(",")
        for op_gpus_str in gpus_strlist:
            op_gpu_list = op_gpus_str.split(",")
            # op_gpu_list == ["-1"] means this op use CPU
            # so don`t check cudavisible.
            if op_gpu_list == ["-1"]:
                continue
            for ids in op_gpu_list:
                if ids not in env_gpus:
                    print("gpu_ids is not in CUDA_VISIBLE_DEVICES.")
                    exit(-1)

    # check gpuid is valid
    for op_gpus_str in gpus_strlist:
        op_gpu_list = op_gpus_str.split(",")
        use_gpu = False
        for ids in op_gpu_list:
            if int(ids) < -1:
                raise ValueError("The input of gpuid error.")
            if int(ids) >= 0:
                use_gpu = True
            if int(ids) == -1 and use_gpu:
                raise ValueError("You can not use CPU and GPU in one model.")

    return gpus_strlist


def is_gpu_mode(unformatted_gpus):
    gpus_strlist = format_gpu_to_strlist(unformatted_gpus)
    for op_gpus_str in gpus_strlist:
        op_gpu_list = op_gpus_str.split(",")
        for ids in op_gpu_list:
            if int(ids) >= 0:
                return True
    return False


def serve_args():
    parser = argparse.ArgumentParser("serve")
    parser.add_argument(
        "server",
        type=str,
        default="start",
        nargs="?",
        help="stop or start PaddleServing, check running environemnt")
    parser.add_argument(
        "--thread",
        type=int,
        default=4,
        help="Concurrency of server,[4,1024]",
        choices=range(4, 1025))
    parser.add_argument(
        "--port", type=int, default=9393, help="Port of the starting gpu")
    parser.add_argument(
        "--device", type=str, default="cpu", help="Type of device")
    parser.add_argument(
        "--gpu_ids", type=str, default="", nargs="+", help="gpu ids")
    parser.add_argument(
        "--runtime_thread_num",
        type=int,
        default=0,
        nargs="+",
        help="Number of each op")
    parser.add_argument(
        "--batch_infer_size",
        type=int,
        default=32,
        nargs="+",
        help="Max batch of each op")
    parser.add_argument(
        "--model", type=str, default="", nargs="+", help="Model for serving")
    parser.add_argument(
        "--op", type=str, default="", nargs="+", help="Model for serving")
    parser.add_argument(
        "--workdir",
        type=str,
        default="workdir",
        help="Working dir of current service")
    parser.add_argument(
        "--use_mkl", default=False, action="store_true", help="Use MKL")
    parser.add_argument(
        "--precision",
        type=str,
        default="fp32",
        help="precision mode(fp32, int8, fp16, bf16)")
    parser.add_argument(
        "--use_calib",
        default=False,
        action="store_true",
        help="Use TensorRT Calibration")
    parser.add_argument(
        "--mem_optim_off",
        default=False,
        action="store_true",
        help="Memory optimize")
    parser.add_argument(
        "--ir_optim", default=False, action="store_true", help="Graph optimize")
    parser.add_argument(
        "--max_body_size",
        type=int,
        default=512 * 1024 * 1024,
        help="Limit sizes of messages")
    parser.add_argument(
        "--use_encryption_model",
        default=False,
        action="store_true",
        help="Use encryption model")
    parser.add_argument(
        "--encryption_rpc_port",
        type=int,
        required=False,
        default=12000,
        help="Port of encryption model, only valid for arg.use_encryption_model")
    parser.add_argument(
        "--use_trt", default=False, action="store_true", help="Use TensorRT")
    parser.add_argument(
        "--use_lite", default=False, action="store_true", help="Use PaddleLite")
    parser.add_argument(
        "--use_xpu", default=False, action="store_true", help="Use XPU")
    parser.add_argument(
        "--use_ascend_cl",
        default=False,
        action="store_true",
        help="Use Ascend CL")
    parser.add_argument(
        "--product_name",
        type=str,
        default=None,
        help="product_name for authentication")
    parser.add_argument(
        "--container_id",
        type=str,
        default=None,
        help="container_id for authentication")
    parser.add_argument(
        "--gpu_multi_stream",
        default=False,
        action="store_true",
        help="Use gpu_multi_stream")
    parser.add_argument(
        "--enable_prometheus",
        default=False,
        action="store_true",
        help="Use Prometheus")
    parser.add_argument(
        "--prometheus_port",
        type=int,
        default=19393,
        help="Port of the Prometheus")
    parser.add_argument(
        "--request_cache_size",
        type=int,
        default=0,
        help="Max request cache size")
    parser.add_argument(
        "--use_dist_model",
        default=False,
        action="store_true",
        help="Use distributed model")
    parser.add_argument(
        "--dist_carrier_id",
        type=str,
        default="",
        help="carrier id of distributed model")
    parser.add_argument(
        "--dist_cfg_file",
        type=str,
        default="",
        help="config file of distributed model")
    parser.add_argument(
        "--dist_endpoints",
        type=str,
        default="+",
        help="endpoints of distributed model. splited by comma")
    parser.add_argument(
        "--dist_nranks",
        type=int,
        default=0,
        help="nranks of distributed model")
    parser.add_argument(
        "--dist_subgraph_index",
        type=int,
        default=-1,
        help="index of distributed model")
    parser.add_argument(
        "--dist_worker_serving_endpoints",
        type=str,
        default=None,
        help="endpoints of worker serving endpoints")
    parser.add_argument(
        "--dist_master_serving",
        default=False,
        action="store_true",
        help="The master serving of distributed inference")
    parser.add_argument(
        "--min_subgraph_size",
        type=str,
        default="",
        nargs="+",
        help="min_subgraph_size")
    parser.add_argument(
        "--gpu_memory_mb",
        type=int,
        default=50,
        help="Initially allocate GPU storage size")
    parser.add_argument(
        "--cpu_math_thread_num",
        type=int,
        default=1,
        help="Initialize the number of CPU computing threads")
    parser.add_argument(
        "--trt_workspace_size",
        type=int,
        default=33554432,
        help="Initialize allocation 1 << 25 GPU storage size")
    parser.add_argument(
        "--trt_use_static",
        default=False,
        action="store_true",
        help="Initialize TRT with static data")

    return parser.parse_args()


def start_gpu_card_model(gpu_mode, port, args):  # pylint: disable=doc-string-missing

    device = "cpu"
    if gpu_mode == True:
        device = "gpu"

    import paddle_serving_server as serving
    op_maker = serving.OpMaker()
    op_seq_maker = serving.OpSeqMaker()
    server = serving.Server()

    thread_num = args.thread
    model = args.model
    mem_optim = args.mem_optim_off is False
    ir_optim = args.ir_optim
    use_mkl = args.use_mkl
    max_body_size = args.max_body_size
    workdir = "{}_{}".format(args.workdir, port)
    dag_list_op = []

    if model == "" and not args.dist_master_serving:
        print("You must specify your serving model")
        exit(-1)
    for single_model_config in args.model:
        if os.path.isdir(single_model_config):
            pass
        elif os.path.isfile(single_model_config):
            raise ValueError("The input of --model should be a dir not file.")

    # 如果通过--op GeneralDetectionOp GeneralRecOp
    # 将不存在的自定义OP加入到DAG图和模型的列表中
    # 并将传入顺序记录在dag_list_op中。
    if args.op != "":
        for single_op in args.op:
            temp_str_list = single_op.split(':')
            if len(temp_str_list) >= 1 and temp_str_list[0] != '':
                if temp_str_list[0] not in op_maker.op_list:
                    op_maker.op_list.append(temp_str_list[0])
                if len(temp_str_list) >= 2 and temp_str_list[1] == '0':
                    pass
                else:
                    server.default_engine_types.append(temp_str_list[0])

                dag_list_op.append(temp_str_list[0])

    # The workflows of master serving in distributed model is different from
    # worker servings. The workflow of worker servings is same to non-distributed
    # model, but workerflow of master serving needs to add IP address of other
    # worker serving in the machine.
    if not args.dist_master_serving:
        read_op = op_maker.create('GeneralReaderOp')
        op_seq_maker.add_op(read_op)
        is_ocr = False
        #如果dag_list_op不是空，那么证明通过--op 传入了自定义OP或自定义的DAG串联关系。
        #此时，根据--op 传入的顺序去组DAG串联关系
        if len(dag_list_op) > 0:
            for single_op in dag_list_op:
                op_seq_maker.add_op(op_maker.create(single_op))
                if single_op == "GeneralDetectionOp":
                    is_ocr = True
        #否则，仍然按照原有方式根虎--model去串联。
        else:
            for idx, single_model in enumerate(model):
                infer_op_name = "GeneralInferOp"
                # 目前由于ocr的节点Det模型依赖于opencv的第三方库
                # 只有使用ocr的时候，才会加入opencv的第三方库并编译GeneralDetectionOp
                # 故此处做特殊处理，当不满足下述情况时，所添加的op默认为GeneralInferOp
                # 以后可能考虑不用python脚本来生成配置
                if len(model
                       ) == 2 and idx == 0 and single_model == "ocr_det_model":
                    infer_op_name = "GeneralDetectionOp"
                    is_ocr = True
                else:
                    infer_op_name = "GeneralInferOp"
                general_infer_op = op_maker.create(infer_op_name)
                op_seq_maker.add_op(general_infer_op)

        general_response_op = op_maker.create('GeneralResponseOp')
        op_seq_maker.add_op(general_response_op)
    else:
        # for the master serving of distributed model only add one general_remote op.
        if args.dist_worker_serving_endpoints is None:
            raise ValueError(
                "Params Error!. dist_worker_serving_endpoints is empty when dist_master_serving is set"
            )
        worker_serving_endpoints = args.dist_worker_serving_endpoints.split(",")
        if len(worker_serving_endpoints) == 0:
            raise ValueError(
                "Params Error!. dist_worker_serving_endpoints is empty when dist_master_serving is set"
            )

        general_remote_op = op_maker.create(
            'GeneralRemoteOp', None, [], [], addresses=worker_serving_endpoints)
        op_seq_maker.add_op(general_remote_op, )

    server.set_op_sequence(op_seq_maker.get_op_sequence())
    server.set_num_threads(thread_num)
    server.use_mkl(use_mkl)
    server.set_precision(args.precision)
    server.set_use_calib(args.use_calib)
    server.set_memory_optimize(mem_optim)
    server.set_ir_optimize(ir_optim)
    server.set_max_body_size(max_body_size)
    server.set_enable_prometheus(args.enable_prometheus)
    server.set_prometheus_port(args.prometheus_port)
    server.set_request_cache_size(args.request_cache_size)
    server.set_enable_dist_model(args.use_dist_model)
    server.set_dist_carrier_id(args.dist_carrier_id)
    server.set_dist_cfg_file(args.dist_cfg_file)
    server.set_dist_nranks(args.dist_nranks)
    server.set_dist_endpoints(args.dist_endpoints.split(","))
    server.set_dist_subgraph_index(args.dist_subgraph_index)
    server.set_min_subgraph_size(args.min_subgraph_size)
    server.set_gpu_memory_mb(args.gpu_memory_mb)
    server.set_cpu_math_thread_num(args.cpu_math_thread_num)

    if args.use_trt and device == "gpu":
        server.set_trt()
        server.set_ir_optimize(True)
        server.set_trt_workspace_size(args.trt_workspace_size)
        server.set_trt_use_static(args.trt_use_static)
        if is_ocr:
            info = set_ocr_dynamic_shape_info()
            server.set_trt_dynamic_shape_info(info)

    if args.gpu_multi_stream and device == "gpu":
        server.set_gpu_multi_stream()

    if args.runtime_thread_num:
        server.set_runtime_thread_num(args.runtime_thread_num)

    if args.batch_infer_size:
        server.set_batch_infer_size(args.batch_infer_size)

    if args.use_lite:
        server.set_lite()

    server.set_device(device)
    if args.use_xpu:
        server.set_xpu()
    if args.use_ascend_cl:
        server.set_ascend_cl()

    if args.product_name != None:
        server.set_product_name(args.product_name)
    if args.container_id != None:
        server.set_container_id(args.container_id)

    if gpu_mode == True or args.use_xpu or args.use_ascend_cl:
        server.set_gpuid(args.gpu_ids)
    server.load_model_config(model)
    server.prepare_server(
        workdir=workdir,
        port=port,
        device=device,
        use_encryption_model=args.use_encryption_model)
    server.run_server()


def set_ocr_dynamic_shape_info():
    info = []
    min_input_shape = {
        "x": [1, 3, 50, 50],
        "conv2d_182.tmp_0": [1, 1, 20, 20],
        "nearest_interp_v2_2.tmp_0": [1, 1, 20, 20],
        "nearest_interp_v2_3.tmp_0": [1, 1, 20, 20],
        "nearest_interp_v2_4.tmp_0": [1, 1, 20, 20],
        "nearest_interp_v2_5.tmp_0": [1, 1, 20, 20]
    }
    max_input_shape = {
        "x": [1, 3, 1536, 1536],
        "conv2d_182.tmp_0": [20, 200, 960, 960],
        "nearest_interp_v2_2.tmp_0": [20, 200, 960, 960],
        "nearest_interp_v2_3.tmp_0": [20, 200, 960, 960],
        "nearest_interp_v2_4.tmp_0": [20, 200, 960, 960],
        "nearest_interp_v2_5.tmp_0": [20, 200, 960, 960],
    }
    opt_input_shape = {
        "x": [1, 3, 960, 960],
        "conv2d_182.tmp_0": [3, 96, 240, 240],
        "nearest_interp_v2_2.tmp_0": [3, 96, 240, 240],
        "nearest_interp_v2_3.tmp_0": [3, 24, 240, 240],
        "nearest_interp_v2_4.tmp_0": [3, 24, 240, 240],
        "nearest_interp_v2_5.tmp_0": [3, 24, 240, 240],
    }
    det_info = {
        "min_input_shape": min_input_shape,
        "max_input_shape": max_input_shape,
        "opt_input_shape": opt_input_shape,
    }
    info.append(det_info)
    min_input_shape = {"x": [1, 3, 32, 10], "lstm_1.tmp_0": [1, 1, 128]}
    max_input_shape = {"x": [50, 3, 32, 1000], "lstm_1.tmp_0": [500, 50, 128]}
    opt_input_shape = {"x": [6, 3, 32, 100], "lstm_1.tmp_0": [25, 5, 128]}
    rec_info = {
        "min_input_shape": min_input_shape,
        "max_input_shape": max_input_shape,
        "opt_input_shape": opt_input_shape,
    }
    info.append(rec_info)
    return info


def start_multi_card(args, serving_port=None):  # pylint: disable=doc-string-missing

    if serving_port == None:
        serving_port = args.port

    if args.use_lite:
        print("run using paddle-lite.")
        start_gpu_card_model(False, serving_port, args)
    else:
        start_gpu_card_model(is_gpu_mode(args.gpu_ids), serving_port, args)


class MainService(BaseHTTPRequestHandler):
    #def __init__(self):
    #    print("MainService ___init________\n")
    def get_available_port(self):
        global encryption_rpc_port
        default_port = encryption_rpc_port
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
            for single_model_config in args.model:
                if os.path.isfile(single_model_config):
                    raise ValueError(
                        "The input of --model should be a dir not file.")
                with open(single_model_config + "/key", "wb") as f:
                    f.write(key)
            return True

    def check_key(self, post_data):
        if "key" not in post_data:
            return False
        else:
            key = base64.b64decode(post_data["key"].encode())
            for single_model_config in args.model:
                if os.path.isfile(single_model_config):
                    raise ValueError(
                        "The input of --model should be a dir not file.")
                with open(single_model_config + "/key", "rb") as f:
                    cur_key = f.read()
                if key != cur_key:
                    return False
            return True

    def start(self, post_data):
        post_data = json.loads(post_data.decode('utf-8'))
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


def stop_serving(command: str, port: int=None):
    '''
    Stop PaddleServing by port.

    Args:
        command(str): stop->SIGINT, kill->SIGKILL
        port(int): Default to None, kill all processes in ProcessInfo.json.
                   Not None, kill the specific process relating to port

    Returns:
         True if stop serving successfully.
         False if error occured

    Examples:
    ..  code-block:: python

        stop_serving("stop", 9494)
    '''
    filepath = os.path.join(CONF_HOME, "ProcessInfo.json")
    infoList = load_pid_file(filepath)
    if infoList is False:
        return False
    lastInfo = infoList[-1]
    for info in infoList:
        storedPort = info["port"]
        pid = info["pid"]
        model = info["model"]
        start_time = info["start_time"]
        if port is not None:
            if port in storedPort:
                kill_stop_process_by_pid(command, pid)
                infoList.remove(info)
                if len(infoList):
                    with open(filepath, "w") as fp:
                        json.dump(infoList, fp)
                else:
                    os.remove(filepath)
                return True
            else:
                if lastInfo == info:
                    raise ValueError(
                        "Please confirm the port [%s] you specified is correct."
                        % port)
                else:
                    pass
        else:
            kill_stop_process_by_pid(command, pid)
            if lastInfo == info:
                os.remove(filepath)
    return True


class Check_Env_Shell(cmd.Cmd):
    intro = "Welcome to the check env shell.Type help to list commands.\n"

    # ----- basic  commands -----
    def do_help(self, arg):
        print("\nCommand list\t\tDescription\n"\
               "check_all\t\tCheck Environment of Paddle Inference, Pipeline Serving, C++ Serving. "\
               "If failed, using debug command to debug\n"\
               "check_pipeline\t\tCheck Environment of Pipeline Serving. "\
               "If failed, using debug command to debug\n"\
               "check_cpp\t\tCheck Environment of C++ Serving. "\
               "If failed, using debug command to debug\n"\
               "check_inference\t\tCheck Environment of Paddle Inference. "\
               "If failed, using debug command to debug\n"\
               "debug\t\t\tWhen checking was failed, open log to debug\n"\
               "exit\t\t\tExit Check Env Shell\n")

    def do_check_all(self, arg):
        "Check Environment of Paddle Inference, Pipeline Serving, C++ Serving"
        check_env("all")

    def do_check_pipeline(self, arg):
        "Check Environment of Pipeline Serving"
        check_env("pipeline")

    def do_check_cpp(self, arg):
        "Check Environment of C++ Serving"
        check_env("cpp")

    def do_check_inference(self, arg):
        "Check Environment of Paddle Inference"
        check_env("inference")

    def do_debug(self, arg):
        "Open pytest log to debug"
        check_env("debug")

    def do_exit(self, arg):
        "Exit Check Env Shell"
        print('Check Environment Shell Exit')
        os._exit(0)
        return True


if __name__ == "__main__":
    # args.device is not used at all.
    # just keep the interface.
    # so --device should not be recommended at the HomePage.
    args = serve_args()
    if args.server == "stop" or args.server == "kill":
        result = 0
        if "--port" in sys.argv:
            result = stop_serving(args.server, args.port)
        else:
            result = stop_serving(args.server)
        if result == 0:
            os._exit(0)
        else:
            os._exit(-1)
    elif args.server == "check":
        Check_Env_Shell().cmdloop()
    for single_model_config in args.model:
        if os.path.isdir(single_model_config):
            pass
        elif os.path.isfile(single_model_config):
            raise ValueError("The input of --model should be a dir not file.")

    if port_is_available(args.port):
        portList = [args.port]
        dump_pid_file(portList, args.model)

    if args.use_encryption_model:
        p_flag = False
        p = None
        serving_port = 0
        encryption_rpc_port = args.encryption_rpc_port
        server = HTTPServer(('localhost', int(args.port)), MainService)
        print(
            'Starting encryption server, waiting for key from client, use <Ctrl-C> to stop'
        )
        server.serve_forever()
    else:
        start_multi_card(args)
