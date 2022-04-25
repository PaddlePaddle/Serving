# coding:utf-8
# Copyright (c) 2020  PaddlePaddle Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License"
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
'''
This module is used to check whether the running environment for PaddleServing is configured correctly.

Two test cases are set for verifying the smooth of environment, fit a line test case for C++ Serving environment and uci for Pipeline Serving enviroment

Usage: export PYTHON_EXECUTABLE=/usr/local/bin/python3.6
       python3.6 -m paddle_serving_server.serve check
'''

import sys
import os
import pytest

inference_test_cases = ["test_fit_a_line.py::TestFitALine::test_inference"]
cpp_test_cases = [
    "test_fit_a_line.py::TestFitALine::test_cpu",
    "test_fit_a_line.py::TestFitALine::test_gpu"
]
pipeline_test_cases = [
    "test_uci_pipeline.py::TestUCIPipeline::test_cpu",
    "test_uci_pipeline.py::TestUCIPipeline::test_gpu"
]
log_files = ["PipelineServingLogs", "log", "stderr.log", "stdout.log"]


def set_serving_log_path():
    if 'SERVING_LOG_PATH' not in os.environ:
        serving_log_path = os.path.expanduser(os.getcwd()) + '/'
        os.environ['SERVING_LOG_PATH'] = serving_log_path


def mv_log_to_new_dir(dir_path):
    import shutil
    if not os.path.exists(dir_path):
        os.mkdir(dir_path)
    serving_log_path = os.environ['SERVING_LOG_PATH']
    for file_name in log_files:
        file_path = os.path.join(serving_log_path, file_name)
        dir_path_temp = os.path.join(dir_path, file_name)
        if os.path.exists(file_path):
            shutil.move(file_path, dir_path_temp)


def run_test_cases(cases_list, case_type, is_open_std):
    old_stdout, old_stderr = sys.stdout, sys.stderr
    real_path = os.path.dirname(os.path.realpath(__file__))
    for case in cases_list:
        if is_open_std is False:
            sys.stdout = open('/dev/null', 'w')
            sys.stderr = open('/dev/null', 'w')
        args_str = "--disable-warnings " + str(real_path) + "/" + case
        args = args_str.split(" ")
        res = pytest.main(args)
        sys.stdout, sys.stderr = old_stdout, old_stderr
        case_name = case.split('_')[-1]
        serving_log_path = os.environ['SERVING_LOG_PATH']
        dir_name = str(case_type) + '_' + case.split(':')[-1]
        new_dir_path = os.path.join(serving_log_path, dir_name)
        mv_log_to_new_dir(new_dir_path)
        if res == 0:
            print("{} {} environment running success".format(case_type,
                                                             case_name))
        elif res == 1:
            if case_name == "inference":
                print(
                    "{} {} environment running failure. Please refer to https://www.paddlepaddle.org.cn/install/quick?docurl=/documentation/docs/zh/install/pip/linux-pip.html to configure environment".
                    format(case_type, case_name))
                os._exit(0)
            else:
                print(
                    "{} {} environment running failure, if you need this environment, please refer to https://github.com/PaddlePaddle/Serving/blob/develop/doc/Install_CN.md".
                    format(case_type, case_name))


def unset_env(key):
    del os.environ[key]


def check_env(mode):
    set_serving_log_path()
    if 'https_proxy' in os.environ or 'http_proxy' in os.environ:
        unset_env("https_proxy")
        unset_env("http_proxy")
    if 'GREP_OPTIONS' in os.environ:
        unset_env("GREP_OPTIONS")
    is_open_std = False
    if mode == "debug":
        is_open_std = True
    if mode == "all" or mode == "inference" or mode == "debug":
        run_test_cases(inference_test_cases, "PaddlePaddle", is_open_std)
    if mode == "all" or mode == "cpp" or mode == "debug":
        run_test_cases(cpp_test_cases, "C++", is_open_std)
    if mode == "all" or mode == "pipeline" or mode == "debug":
        run_test_cases(pipeline_test_cases, "Pipeline", is_open_std)


if __name__ == '__main__':
    check_env("debug")
