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
    Convert a paddle inference model into a model file that can be used for Paddle Serving.
    Example:
        python -m paddle_serving_client.convert --dirname ./inference_model
"""
import argparse
from .io import inference_model_to_serving


def parse_args():  # pylint: disable=doc-string-missing
    parser = argparse.ArgumentParser("convert")
    parser.add_argument(
        "--show_proto",
        type=bool,
        default=False,
        help='If yes, you can preview the proto and then determine your feed var alias name and fetch var alias name.'
    )
    parser.add_argument(
        "--dirname",
        type=str,
        required=True,
        help='Path of saved model files. Program file and parameter files are saved in this directory.'
    )
    parser.add_argument(
        "--serving_server",
        type=str,
        default="serving_server",
        help='The path of model files and configuration files for server. Default: "serving_server".'
    )
    parser.add_argument(
        "--serving_client",
        type=str,
        default="serving_client",
        help='The path of configuration files for client. Default: "serving_client".'
    )
    parser.add_argument(
        "--model_filename",
        type=str,
        default=None,
        help='The name of file to load the inference program. If it is None, the default filename __model__ will be used'
    )
    parser.add_argument(
        "--params_filename",
        type=str,
        default=None,
        help='The name of file to load all parameters. It is only used for the case that all parameters were saved in a single binary file. If parameters were saved in separate files, set it as None. Default: None.'
    )
    parser.add_argument(
        "--feed_alias_names",
        type=str,
        default=None,
        help='set alias names for feed vars, split by comma \',\', you should run --show_proto to check the number of feed vars'
    )
    parser.add_argument(
        "--fetch_alias_names",
        type=str,
        default=None,
        help='set alias names for feed vars, split by comma \',\', you should run --show_proto to check the number of fetch vars'
    )
    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()
    inference_model_to_serving(
        args.dirname,
        serving_server=args.serving_server,
        serving_client=args.serving_client,
        model_filename=args.model_filename,
        params_filename=args.params_filename,
        show_proto=args.show_proto,
        feed_alias_names=args.feed_alias_names,
        fetch_alias_names=args.fetch_alias_names)
