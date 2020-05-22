#   Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
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
    Download a package for serving directly
    Example:
        python -m paddle_serving_app.models --get_model senta_bilstm
        python -m paddle_serving_app.models --list_model
"""

import argparse
import sys
from .models import ServingModels


def parse_args():  # pylint: disable=doc-string-missing
    parser = argparse.ArgumentParser("serve")
    parser.add_argument(
        "--get_model", type=str, default="", help="Download a specific model")
    parser.add_argument(
        '--list_model', nargs='*', default=None, help="List Models")
    parser.add_argument(
        '--tutorial', type=str, default="", help="Get running command")
    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()
    if args.list_model != None:
        model_handle = ServingModels()
        model_dict = model_handle.get_model_list()
        # Task level model list
        # Text Classification, Semantic Representation
        # Image Classification, Object Detection, Image Segmentation
        for key in model_dict:
            print("-----------------------------------------------")
            print("{}: {}".format(key, " | ".join(model_dict[key])))

    elif args.get_model != "":
        model_handle = ServingModels()
        model_dict = model_handle.url_dict
        if args.get_model not in model_dict:
            print(
                "Your model name does not exist in current model list, stay tuned"
            )
            sys.exit(0)
        model_handle.download(args.get_model)
    elif args.tutorial != "":
        model_handle = ServingModels()
        model_dict = model_handle.url_dict
        if args.get_model not in model_dict:
            print(
                "Your model name does not exist in current model list, stay tuned"
            )
            sys.exit(0)
        tutorial_str = model_handle.get_tutorial()
        print(tutorial_str)
    else:
        print("Wrong argument")
        print("""
              Usage:
              Download a package for serving directly
              Example:
                   python -m paddle_serving_app.models --get_model senta_bilstm
                   python -m paddle_serving_app.models --list_model
              """)
        pass
