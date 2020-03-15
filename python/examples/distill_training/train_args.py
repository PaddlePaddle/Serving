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

import os
import sys
import argparse


def train_args(yml_file=""):
    parser = argparse.ArgumentParser("trainer")
    parser.add_argument(
        "--base_lr", type=float, default=0.001, help="base learning rate")
    parser.add_argument(
        "--vocab_size", type=int, default=17964, help="size of vocab")
    parser.add_argument(
        "--num_labels", type=int, default=57, help="dim of output labels")
    parser.add_argument(
        "--word_emb_dim", type=int, default=128, help="dim of embedding")
    parser.add_argument(
        "--grnn_hidden_dim", type=int, default=128, help="dim of hidden layer")
    parser.add_argument(
        "--bigru_num", type=int, default=2, help="number of gru")
    parser.add_argument(
        "--infer", type=bool, default=False, help="whether do infer only")
    parser.add_argument(
        "--backbone", type=str, default="bigru", help="backbone of model")
    parser.add_argument("--thread", type=int, default=10, help="concurrency")
    parser.add_argument(
        "--endpoint_list",
        type=str,
        #default="127.0.0.1:9295-127.0.0.1:9296-127.0.0.1:9297-127.0.0.1:9298",
        default="127.0.0.1:9296",
        help="endpoint of reader server")
    parse_result = parser.parse_args()
    if yml_file != "":
        # parse yml here
        pass
    return parse_result
