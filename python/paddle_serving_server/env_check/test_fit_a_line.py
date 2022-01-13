import os
import subprocess
import numpy as np
import copy
import cv2
import re
import sys

from paddle_serving_client import Client
from paddle_serving_client.httpclient import HttpClient
from paddle_serving_app.reader import *

from util import *


class TestFitALine(object):
    def setup_class(self):
        serving_util = ServingTest(data_path="fit_a_line", example_path="fit_a_line", model_dir="uci_housing_model",
                                   client_dir="uci_housing_client")
        serving_util.check_model_data_exist()
        self.get_truth_val_by_inference(self)
        self.serving_util = serving_util
        self.serving_util.release('service')
        kill_process(9494)

    def teardown_method(self):
        print_log(["stderr.log", "stdout.log",
                   "log/serving.ERROR", "PipelineServingLogs/pipeline.log"])
        kill_process(9494)
        self.serving_util.release('service')

    def get_truth_val_by_inference(self):
        try:
            import paddle.inference as paddle_infer
        except:
            # when paddle is not installed, directly return
            return
        data = np.array(
            [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795,
             -0.0332]).astype("float32")[np.newaxis, :]
        input_dict = {"x": data}

        pd_config = paddle_infer.Config("uci_housing_model/")
        pd_config.disable_gpu()
        pd_config.switch_ir_optim(False)

        predictor = paddle_infer.create_predictor(pd_config)

        input_names = predictor.get_input_names()
        for i, input_name in enumerate(input_names):
            input_handle = predictor.get_input_handle(input_name)
            input_handle.copy_from_cpu(input_dict[input_name])

        predictor.run()

        output_data_dict = {}
        output_names = predictor.get_output_names()
        for _, output_data_name in enumerate(output_names):
            output_handle = predictor.get_output_handle(output_data_name)
            output_data = output_handle.copy_to_cpu()
            output_data_dict[output_data_name] = output_data
        # convert to the same format of Serving output
        output_data_dict["price"] = output_data_dict["fc_0.tmp_1"]
        del output_data_dict["fc_0.tmp_1"]
        self.truth_val = output_data_dict

    def predict_brpc(self, batch_size=1):
        data = np.array(
            [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795,
             -0.0332]).astype("float32")[np.newaxis, :]
        client = Client()
        client.load_client_config("uci_housing_client/serving_client_conf.prototxt")
        client.connect(["127.0.0.1:9494"])
        fetch_list = client.get_fetch_names()
        fetch_map = client.predict(
            feed={"x": data}, fetch=fetch_list, batch=True)
        return fetch_map
    
    def predict_http(self, batch_size=1):
        data = np.array(
            [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795,
             -0.0332]).astype("float32")[np.newaxis, :]
        client = HttpClient()
        client.load_client_config("uci_housing_client/serving_client_conf.prototxt")
        client.connect(["127.0.0.1:9494"])
        fetch_list = client.get_fetch_names()
        fetch_map = client.predict(
            feed={"x": data}, fetch=fetch_list, batch=True)
        output_dict = self.serving_util.parse_http_result(fetch_map)
        return output_dict

    def test_inference(self):
        self.serving_util.start_server_by_shell(cmd="", sleep=1)
        assert self.truth_val['price'].size != 0, "The result of inference is empty"


    def test_cpu(self):
        # 1.start server
        self.serving_util.start_server_by_shell(
            cmd=f"{self.serving_util.py_version} -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9494",
            sleep=5,
        )

        # 2.resource check
        assert count_process_num_on_port(9494) == 1, "Error occured when Paddle Server started"

        # 4.predict by brpc
        # batch_size 1
        result_data = self.predict_brpc()
        self.serving_util.check_result(result_data=result_data, truth_data=self.truth_val, batch_size=1)
        result_data = self.predict_http()
        self.serving_util.check_result(result_data=result_data, truth_data=self.truth_val, batch_size=1)

        # 5.release
        kill_process(9494)

    def test_gpu(self):
        # 1.start server
        self.serving_util.start_server_by_shell(
            cmd=f"{self.serving_util.py_version} -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9494 --gpu_ids 0",
            sleep=5,
        )

        # 2.resource check
        assert count_process_num_on_port(9494) == 1, "Error occured when Paddle Server started"

        # 4.predict by brpc 
        # batch_size 1
        result_data = self.predict_brpc()
        self.serving_util.check_result(result_data=result_data, truth_data=self.truth_val, batch_size=1)
        result_data = self.predict_http()
        self.serving_util.check_result(result_data=result_data, truth_data=self.truth_val, batch_size=1)

        # 5.release
        kill_process(9494)


