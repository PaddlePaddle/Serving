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
import os
from multiprocessing import Pool, Process
from .web_service import WebService
import paddle_serving_server_gpu as serving
from paddle_serving_server_gpu import serve_args

if __name__ == "__main__":
    args = serve_args()
    web_service = WebService(name=args.name)
    web_service.load_model_config(args.model)
    gpu_ids = []
    if args.gpu_ids == "":
        if "CUDA_VISIBLE_DEVICES" in os.environ:
            gpu_ids = os.environ["CUDA_VISIBLE_DEVICES"]
    if len(gpu_ids) > 0:
        gpus = [int(x) for x in gpu_ids.split(",")]
        web_service.set_gpus(gpus)
    web_service.prepare_server(
        workdir=args.workdir, port=args.port, device=args.device)
    web_service.run_server()
