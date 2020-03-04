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
        python -m paddle_serving_server.web_serve --model ./serving_server_model --port 9292
"""
import argparse
from multiprocessing import Pool, Process
from .web_service import WebService

def parse_args():
    parser = argparse.ArgumentParser("web_serve")
    parser.add_argument("--thread", type=int, default=10, help="Concurrency of server")
    parser.add_argument("--model", type=str, default="", help="Model for serving")
    parser.add_argument("--port", type=int, default=9292, help="Port the server")
    parser.add_argument("--workdir", type=str, default="workdir", help="Working dir of current service")
    parser.add_argument("--device", type=str, default="cpu", help="Type of device")
    parser.add_argument("--name", type=str, default="default", help="Default service name")
    return parser.parse_args()

if __name__ == "__main__":
    args = parse_args()
    service = WebService(name=args.name)
    service.load_model_config(args.model)
    service.prepare_server(workdir=args.workdir, port=args.port, device=args.device)
    service.run_server()
