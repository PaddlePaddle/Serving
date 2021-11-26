import os
import subprocess
import numpy as np
import copy
import cv2
import sys
from paddle_serving_client import Client, HttpClient
from paddle_serving_app.reader import LACReader
import paddle.inference as paddle_infer

from util import *


class TestLAC(object):
    def setup_class(self):
        serving_util = ServingTest(data_path="lac", example_path="lac", model_dir="lac_model",
                                   client_dir="lac_client")
        serving_util.check_model_data_exist()
        self.get_truth_val_by_inference(self)
        self.serving_util = serving_util

    def teardown_method(self):
        print_log(["stderr.log", "stdout.log",
                   "log/serving.ERROR", "PipelineServingLogs/pipeline.log"], iden="after predict")
        kill_process(9293)
        self.serving_util.release()

    def get_truth_val_by_inference(self):
        reader = LACReader()
        line = "我爱北京天安门"
        feed_data = reader.process(line)
        input_dict = {
            "word": np.array(feed_data + feed_data).reshape(len(feed_data) * 2, 1),
            "word.lod": [0, len(feed_data), 2 * len(feed_data)]
        }

        pd_config = paddle_infer.Config("lac_model")
        pd_config.disable_gpu()
        pd_config.switch_ir_optim(False)

        predictor = paddle_infer.create_predictor(pd_config)

        input_names = predictor.get_input_names()
        for i, input_name in enumerate(input_names):
            input_handle = predictor.get_input_handle(input_name)
            # 设置变长tensor
            input_handle.set_lod([input_dict[f"{input_name}.lod"]])
            input_handle.copy_from_cpu(input_dict[input_name])

        predictor.run()

        output_data_dict = {}
        output_names = predictor.get_output_names()
        for _, output_data_name in enumerate(output_names):
            output_handle = predictor.get_output_handle(output_data_name)
            output_data = output_handle.copy_to_cpu()
            output_data_dict[output_data_name] = output_data
        # 对齐Serving output
        output_data_dict["crf_decode"] = output_data_dict["save_infer_model/scale_0"]
        del output_data_dict["save_infer_model/scale_0"]
        self.truth_val = output_data_dict
        print(self.truth_val, self.truth_val["crf_decode"].shape)

    def predict_brpc(self, batch_size=2):
        reader = LACReader()
        line = "我爱北京天安门"
        feed_data = reader.process(line)
        feed_dict = {
            "words": np.array(feed_data + feed_data).reshape(len(feed_data) * 2, 1),
            "words.lod": [0, len(feed_data), 2 * len(feed_data)]
        }

        fetch = ["crf_decode"]
        endpoint_list = ['127.0.0.1:9293']

        client = Client()
        client.load_client_config(self.serving_util.client_config)
        client.connect(endpoint_list)

        fetch_map = client.predict(feed=feed_dict, fetch=fetch, batch=True)
        print(fetch_map)
        return fetch_map

    def predict_http(self, mode="proto", compress=False, batch_size=2):
        reader = LACReader()
        line = "我爱北京天安门"
        feed_data = reader.process(line)
        feed_dict = {
            "words": np.array(feed_data + feed_data).reshape(len(feed_data) * 2, 1),
            "words.lod": [0, len(feed_data), 2 * len(feed_data)]
        }

        fetch = ["crf_decode"]

        client = HttpClient()
        client.load_client_config(self.serving_util.client_config)
        if mode == "proto":
            client.set_http_proto(True)
        elif mode == "json":
            client.set_http_proto(False)
        elif mode == "grpc":
            client.set_use_grpc_client(True)
        else:
            exit(-1)
        if compress:
            client.set_response_compress(True)
            client.set_request_compress(True)
        client.connect(["127.0.0.1:9293"])

        fetch_map = client.predict(feed=feed_dict, fetch=fetch, batch=True)
        result_dict = {}
        print(fetch_map)
        if isinstance(fetch_map, dict):
            for tensor in fetch_map["outputs"][0]["tensor"]:
                result_dict[tensor["alias_name"]] = np.array(tensor["int64_data"]).reshape(tensor["shape"])
        else:
            for tensor in fetch_map.outputs[0].tensor:
                result_dict[tensor.alias_name] = np.array(tensor.int64_data).reshape(tensor.shape)
        print(result_dict)
        return result_dict

    def test_cpu(self):
        # 1.start server
        self.serving_util.start_server_by_shell(
            cmd=f"{self.serving_util.py_version} -m paddle_serving_server.serve --model lac_model --port 9293",
            sleep=5,
        )

        # 2.resource check
        #assert count_process_num_on_port(9293) == 1
        #assert check_gpu_memory(2) is False

        # 3.keywords check

        # 4.predict by brpc
        # batch_size 2
        result_data = self.predict_brpc(batch_size=2)
        # 删除lod信息
        del result_data["crf_decode.lod"]
        self.serving_util.check_result(result_data=result_data, truth_data=self.truth_val, batch_size=1, delta=1)
        # predict by http
        # batch_size 2
        result_data = self.predict_http(mode="proto", batch_size=2)
        self.serving_util.check_result(result_data=result_data, truth_data=self.truth_val, batch_size=1, delta=1)
        result_data = self.predict_http(mode="json", batch_size=1)
        self.serving_util.check_result(result_data=result_data, truth_data=self.truth_val, batch_size=1, delta=1)
        result_data = self.predict_http(mode="grpc", batch_size=1)
        self.serving_util.check_result(result_data=result_data, truth_data=self.truth_val, batch_size=1, delta=1)
        # # compress
        result_data = self.predict_http(mode="proto", compress=True, batch_size=1)
        self.serving_util.check_result(result_data=result_data, truth_data=self.truth_val, batch_size=1, delta=1)
        result_data = self.predict_http(mode="json", compress=True, batch_size=1)
        self.serving_util.check_result(result_data=result_data, truth_data=self.truth_val, batch_size=1, delta=1)

        # 5.release
        kill_process(9293)

    def test_gpu(self):
        # 1.start server
        self.serving_util.start_server_by_shell(
            cmd=f"{self.serving_util.py_version} -m paddle_serving_server.serve --model lac_model --port 9293 --gpu_ids 0",
            sleep=8,
        )

        # 2.resource check
        assert count_process_num_on_port(9293) == 1
        #assert check_gpu_memory(3) is True
        #assert check_gpu_memory(1) is False

        # 3.keywords check
        check_keywords_in_server_log("Sync params from CPU to GPU", filename="stderr.log")

        # 4.predict by brpc
        # batch_size 2
        result_data = self.predict_brpc(batch_size=2)
        # 删除lod信息
        del result_data["crf_decode.lod"]
        self.serving_util.check_result(result_data=result_data, truth_data=self.truth_val, batch_size=1, delta=1)
        # predict by http
        # batch_size 2
        result_data = self.predict_http(mode="proto", batch_size=2)
        self.serving_util.check_result(result_data=result_data, truth_data=self.truth_val, batch_size=1, delta=1)
        result_data = self.predict_http(mode="json", batch_size=1)
        self.serving_util.check_result(result_data=result_data, truth_data=self.truth_val, batch_size=1, delta=1)
        result_data = self.predict_http(mode="grpc", batch_size=1)
        self.serving_util.check_result(result_data=result_data, truth_data=self.truth_val, batch_size=1, delta=1)
        # # compress
        result_data = self.predict_http(mode="proto", compress=True, batch_size=1)
        self.serving_util.check_result(result_data=result_data, truth_data=self.truth_val, batch_size=1, delta=1)
        result_data = self.predict_http(mode="json", compress=True, batch_size=1)
        self.serving_util.check_result(result_data=result_data, truth_data=self.truth_val, batch_size=1, delta=1)

        # 5.release
        kill_process(9293, 2)

