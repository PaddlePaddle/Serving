import pytest
import sys
import os

cpp_test_cases = ["test_lac.py::TestLAC::test_cpu", "test_lac.py::TestLAC::test_gpu"]
pipeline_test_cases = ["test_uci_pipeline.py::TestUCIPipeline::test_cpu", "test_uci_pipeline.py::TestUCIPipeline::test_gpu"]

def run_test_cases(cases_list, case_type):
    old_stdout, old_stderr = sys.stdout, sys.stderr
    real_path = os.path.dirname(os.path.realpath(__file__))
    for case in cases_list:
        sys.stdout = open('/dev/null', 'w')
        sys.stderr = open('/dev/null', 'w')
        args_str = "--disable-warnings " + str(real_path) + "/" + case
        args = args_str.split(" ")
        res = pytest.main(args)
        sys.stdout, sys.stderr = old_stdout, old_stderr
        if res == 0:
            print("{} {} environment running success".format(case_type, case[-3:]))
        else:
            print("{} {} environment running failure, if you need this environment, please refer to https://github.com/PaddlePaddle/Serving/blob/v0.7.0/doc/Install_CN.md to configure environment".format(case_type, case[-3:]))
    
def unset_proxy(key):
    os.unsetenv(key)

def check_env():
    if 'https_proxy' in os.environ or 'http_proxy' in os.environ:
        unset_proxy("https_proxy") 
        unset_proxy("http_proxy")     
    run_test_cases(cpp_test_cases, "C++")
    run_test_cases(pipeline_test_cases, "Pipeline")
