# Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
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
import re
import tarfile
import paddle_gpu_serving
import subprocess
import imp
import time
import socket
from contextlib import closing


class BertServer():
    def __init__(self, with_gpu=True):
        try:
            imp.find_module('paddlehub')
            self.paddlehub_found = True
            print('Working with paddlehub')
        except ImportError:
            self.paddlehub_found = False
        os.chdir(self.get_path())
        self.with_gpu_flag = with_gpu
        self.p_list = []
        self.use_other_model = False
        self.run_m = False
        self.model_url = 'https://paddle-serving.bj.bcebos.com/data/bert'
        self.bin_url = 'https://paddle-serving.bj.bcebos.com/paddle-gpu-serving/bin'
        self.cpu_run_cmd = './bin/serving-cpu --logtostderr=true '
        self.gpu_run_cmd = './bin/serving-gpu --bthread_min_concurrency=4 --bthread_concurrency=4 --logtostderr=true '
        self.model_path_str = ''
        self.get_exe()

    def get_exe(self):
        exe_path = './bin'
        module_version = paddle_gpu_serving.__version__
        target_version_list = module_version.strip().split('.')
        target_version = target_version_list[0] + '.' + target_version_list[1]
        need_download = False

        if os.path.exists(exe_path):
            with open('./bin/serving-version.txt') as f:
                serving_version = f.read().strip()
            if serving_version != target_version:
                need_download = True
        else:
            need_download = True
        if need_download:
            tar_name = 'paddle-gpu-serving-' + target_version + '-bin.tar.gz'
            bin_url = self.bin_url + '/' + tar_name
            print('Frist time run, downloading PaddleServing components ...')
            r = os.system('wget ' + bin_url + ' --no-check-certificate')
            if r != 0:
                print('Download failed')
                if os.path.exists(tar_name):
                    os.remove(tar_name)
            else:
                try:
                    print('Decompressing files ..')
                    tar = tarfile.open(tar_name)
                    tar.extractall()
                    tar.close()
                except:
                    if os.path.exists(exe_path):
                        os.remove(exe_path)
                finally:
                    os.remove(tar_name)

    def build_server(self):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.bind(('0.0.0.0', self.port))
        sock.listen(5)
        print('Main server serving on {} port.'.format(self.port))
        while True:
            con, addr = sock.accept()
            request = con.recv(1024)
            response = 'status:0\tmodel name:' + str(
                self.model_name) + '\t' + 'serving port:' + str(
                    self.serving_port)

            con.send(bytes(response, encoding='utf-8'))
            con.close()

    def modify_conf(self, gpu_index=0):
        os.chdir(self.get_path())
        if not self.with_gpu_flag:
            with open('./conf/model_toolkit.prototxt', 'r') as f:
                conf_str = f.read()
            conf_str = re.sub('GPU', 'CPU', conf_str)
            conf_str = re.sub('model_data_path.*"', self.model_path_str,
                              conf_str)
            conf_str = re.sub('enable_memory_optimization: 0',
                              'enable_memory_optimization: 1', conf_str)
            open('./conf/model_toolkit.prototxt', 'w').write(conf_str)

        else:
            conf_file = './conf/model_toolkit.prototxt.' + str(gpu_index)
            with open(conf_file, 'r') as f:
                conf_str = f.read()
            conf_str = re.sub('CPU', 'GPU', conf_str)
            conf_str = re.sub('model_data_path.*"', self.model_path_str,
                              conf_str)
            conf_str = re.sub('enable_memory_optimization: 0',
                              'enable_memory_optimization: 1', conf_str)
            open(conf_file, 'w').write(conf_str)

    def find_serving_port(self):
        for i in range(1000):
            port = 9000 + i
            with closing(socket.socket(socket.AF_INET,
                                       socket.SOCK_STREAM)) as sock:
                sock.settimeout(2)
                result = sock.connect_ex(('0.0.0.0', port))
            if result != 0:
                return port
        return -1

    def hold(self):
        try:
            self.build_server()
        except KeyboardInterrupt:
            print("Server is going to quit")
            time.sleep(5)

    def run(self, gpu_index=0, port=8866):

        self.port = port
        os.chdir(self.get_path())
        self.modify_conf(gpu_index)
        serving_port = self.find_serving_port()
        if serving_port < 0:
            print('No port available.')
            return -1
        self.serving_port = serving_port

        if self.with_gpu_flag == True:
            gpu_msg = '--gpuid=' + str(gpu_index) + ' '
            run_cmd = self.gpu_run_cmd + gpu_msg
            run_cmd += '--port=' + str(
                serving_port) + ' ' + '--resource_file=resource.prototxt.' + str(
                    gpu_index) + ' '
            print('Start serving on gpu ' + str(gpu_index) + ' port = ' + str(
                serving_port))
        else:
            re = subprocess.Popen(
                'cat /usr/local/cuda/version.txt > tmp 2>&1', shell=True)
            re.wait()
            if re.returncode == 0:
                run_cmd = self.gpu_run_cmd + '--port=' + str(serving_port) + ' '
            else:
                run_cmd = self.cpu_run_cmd + '--port=' + str(serving_port) + ' '
            print('Start serving on cpu port = {}'.format(serving_port))

        process = subprocess.Popen(run_cmd, shell=True)

        self.p_list.append(process)
        if not self.run_m:
            self.hold()

    def run_multi(self, gpu_index_list=[], port_list=[]):
        self.run_m = True
        if len(port_list) < 1:
            print('Please set one port at least.')
            return -1
        if self.with_gpu_flag == True:
            if len(gpu_index_list) != len(port_list):
                print('Expect same length of gpu_index_list and port_list.')
                return -1
            for gpu_index, port in zip(gpu_index_list, port_list):
                self.run(gpu_index=gpu_index, port=port)
        else:
            for port in port_list:
                self.run(port=port)
        self.hold()

    def stop(self):
        for p in self.p_list:
            p.kill()

    def with_model(self, model_name=None, model_url=None):
        if model_name == None or type(model_name) != str:
            print('Please set model name string')
        self.model_name = model_name
        os.chdir(self.get_path())
        self.get_model(model_name)

    def get_path(self):
        py_path = os.path.dirname(paddle_gpu_serving.__file__)
        server_path = os.path.join(py_path, 'server')
        return server_path

    def get_model(self, model_name):
        server_path = self.get_path()
        if not self.paddlehub_found or self.use_other_model:
            tar_name = model_name + '.tar.gz'
            model_url = self.model_url + '/' + tar_name

            model_path = os.path.join(server_path, 'data/model/paddle/fluid')
            if not os.path.exists(model_path):
                os.makedirs('data/model/paddle/fluid')
            os.chdir(model_path)
            if os.path.exists(model_name):
                pass
            else:
                os.system('wget ' + model_url + ' --no-check-certificate')
                print('Decompressing files ..')
                tar = tarfile.open(tar_name)
                tar.extractall()
                tar.close()
                os.remove(tar_name)

            self.model_path_str = r'model_data_path: "./data/model/paddle/fluid/' + model_name + r'"'

        else:
            import paddlehub as hub
            import paddle.fluid as fluid

            paddlehub_modules_path = os.path.expanduser('~/.paddlehub')
            paddlehub_bert_path = os.path.join(paddlehub_modules_path,
                                               'bert_service')
            model_path = os.path.join(paddlehub_bert_path, model_name)
            self.model_path_str = r'model_data_path: "' + model_path + r'"'

            if not os.path.exists(model_path):
                print('Save model for serving ...')
                module = hub.Module(name=model_name)
                inputs, outputs, program = module.context(
                    trainable=True, max_seq_len=128)
                place = fluid.core_avx.CPUPlace()
                exe = fluid.Executor(place)
                input_ids = inputs["input_ids"]
                position_ids = inputs["position_ids"]
                segment_ids = inputs["segment_ids"]
                input_mask = inputs["input_mask"]
                feed_var_names = [
                    input_ids.name, position_ids.name, segment_ids.name,
                    input_mask.name
                ]
                target_vars = [
                    outputs["pooled_output"], outputs["sequence_output"]
                ]
                os.makedirs(model_path)
                fluid.io.save_inference_model(
                    feeded_var_names=feed_var_names,
                    target_vars=target_vars,
                    main_program=program,
                    executor=exe,
                    dirname=model_path)

        os.chdir(self.get_path())
