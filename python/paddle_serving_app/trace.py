# -*- coding: utf-8 -*-
"""
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

import json
import sys
import argparse


def parse_args():
    parser = argparse.ArgumentParser("Convert profile log to trace")
    parser.add_argument(
        "--profile_file",
        type=str,
        default="",
        required=True,
        help="Profile log")
    parser.add_argument(
        "--trace_file", type=str, default="trace", help="Trace file")
    return parser.parse_args()


def prase(pid_str, time_str, counter):
    pid = pid_str.split(":")[1]
    event_list = time_str.split(" ")
    trace_list = []
    for event in event_list:
        name, ts = event.split(":")
        name_list = name.split("_")
        ph = "B" if (name_list[-1] == "0") else "E"
        if len(name_list) == 2:
            name = name_list[0]
        else:
            name = name_list[0] + "_" + name_list[1]
        event_dict = {}
        event_dict["name"] = name
        event_dict["tid"] = 0
        event_dict["pid"] = pid
        event_dict["ts"] = ts
        event_dict["ph"] = ph

        trace_list.append(event_dict)
    return trace_list


if __name__ == "__main__":
    args = parse_args()
    profile_file = args.profile_file
    trace_file = args.trace_file
    all_list = []
    counter = 0
    with open(profile_file) as f:
        for line in f.readlines():
            line = line.strip().split("\t")
            if line[0] == "PROFILE":
                trace_list = prase(line[1], line[2], counter)
                counter += 1
                for trace in trace_list:
                    all_list.append(trace)

    trace = json.dumps(all_list, indent=2, separators=(',', ':'))
    with open(trace_file, "w") as f:
        f.write(trace)
