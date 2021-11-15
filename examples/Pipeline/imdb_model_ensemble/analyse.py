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

from paddle_serving_server.pipeline import Analyst
import json
import logging
import sys

logging.basicConfig(level=logging.INFO)

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python analyse.py <log_filename> <trace_filename>")
        exit(1)
    log_filename = sys.argv[1]
    trace_filename = sys.argv[2]
    analyst = Analyst(log_filename)
    analyst.save_trace(trace_filename)
    op_analyst = analyst.get_op_analyst()
    op_concurrency = op_analyst.concurrency_analysis("analyse.yaml")
    print(json.dumps(op_concurrency, indent=2, separators=(',', ':')))
