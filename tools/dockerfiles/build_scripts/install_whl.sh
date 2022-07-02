#!/bin/bash

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

SERVING_VERSION=$1
PADDLE_VERSION=$2
RUN_ENV=$3 # cpu/10.1 10.2
PYTHON_VERSION=$4
serving_release=
client_release="paddle-serving-client==$SERVING_VERSION"
app_release="paddle-serving-app==$SERVING_VERSION"

if [[ $PYTHON_VERSION == "3.6" ]];then
    CPYTHON="36"
    CPYTHON_PADDLE="36m"
elif [[ $PYTHON_VERSION == "3.7" ]];then
    CPYTHON="37"
    CPYTHON_PADDLE="37m"
elif [[ $PYTHON_VERSION == "3.8" ]];then
    CPYTHON="38"
    CPYTHON_PADDLE="38"
fi

if [[ "$RUN_ENV" == "cpu" ]];then
  server_release="https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server-$SERVING_VERSION-py3-none-any.whl"
  serving_bin="https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-cpu-avx-mkl-$SERVING_VERSION.tar.gz"
  paddle_whl="paddlepaddle==$PADDLE_VERSION"
elif [[ "$RUN_ENV" == "cuda10.1" ]];then
  server_release="https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server_gpu-$SERVING_VERSION.post101-py3-none-any.whl"
  serving_bin="https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-gpu-101-$SERVING_VERSION.tar.gz"
  paddle_whl="https://paddle-inference-lib.bj.bcebos.com/$PADDLE_VERSION/python/Linux/GPU/x86-64_gcc8.2_avx_mkl_cuda10.1_cudnn7.6.5_trt6.0.1.5/paddlepaddle_gpu-$PADDLE_VERSION.post101-cp$CPYTHON-cp$CPYTHON_PADDLE-linux_x86_64.whl"
elif [[ "$RUN_ENV" == "cuda10.2" ]] ;then
  server_release="https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server_gpu-$SERVING_VERSION.post1028-py3-none-any.whl"
  serving_bin="https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-gpu-1028-$SERVING_VERSION.tar.gz"
  paddle_whl="https://paddle-inference-lib.bj.bcebos.com/$PADDLE_VERSION/python/Linux/GPU/x86-64_gcc8.2_avx_mkl_cuda10.2_cudnn8.1.1_trt7.2.3.4/paddlepaddle_gpu-$PADDLE_VERSION-cp$CPYTHON-cp$CPYTHON_PADDLE-linux_x86_64.whl"
elif [[ "$RUN_ENV" == "cuda11.2" ]];then
  server_release="https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server_gpu-$SERVING_VERSION.post112-py3-none-any.whl"
  serving_bin="https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-gpu-112-$SERVING_VERSION.tar.gz"
  paddle_whl="https://paddle-inference-lib.bj.bcebos.com/$PADDLE_VERSION/python/Linux/GPU/x86-64_gcc8.2_avx_mkl_cuda11.2_cudnn8.2.1_trt8.0.3.4/paddlepaddle_gpu-$PADDLE_VERSION.post112-cp$CPYTHON-cp$CPYTHON_PADDLE-linux_x86_64.whl"
fi

client_release="https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_client-$SERVING_VERSION-cp$CPYTHON-none-any.whl"
app_release="https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_app-$SERVING_VERSION-py3-none-any.whl"


if [[ "$RUN_ENV" == "cpu" ]];then
    python$PYTHON_VERSION -m pip install $client_release $app_release $server_release
    python$PYTHON_VERSION -m pip install $paddle_whl
    cd /usr/local/
    wget $serving_bin 
    tar xf serving-cpu-avx-mkl-${SERVING_VERSION}.tar.gz
    mv $PWD/serving-cpu-avx-mkl-${SERVING_VERSION} $PWD/serving_bin
    echo "export SERVING_BIN=$PWD/serving_bin/serving">>/root/.bashrc
    rm -rf serving-cpu-avx-mkl-${SERVING_VERSION}.tar.gz
    cd -
elif [[ "$RUN_ENV" == "cuda10.1" ]];then
    python$PYTHON_VERSION -m pip install $client_release $app_release $server_release
    python$PYTHON_VERSION -m pip install $paddle_whl
    cd /usr/local/
    wget $serving_bin
    tar xf serving-gpu-101-${SERVING_VERSION}.tar.gz
    mv $PWD/serving-gpu-101-${SERVING_VERSION} $PWD/serving_bin
    echo "export SERVING_BIN=$PWD/serving_bin/serving">>/root/.bashrc
    rm -rf serving-gpu-101-${SERVING_VERSION}.tar.gz
    cd -
elif [[ "$RUN_ENV" == "cuda10.2" ]];then
    python$PYTHON_VERSION -m pip install $client_release $app_release $server_release
    python$PYTHON_VERSION -m pip install $paddle_whl
    cd /usr/local/
    wget $serving_bin
    tar xf serving-gpu-102-${SERVING_VERSION}.tar.gz
    mv $PWD/serving-gpu-102-${SERVING_VERSION} $PWD/serving_bin
    echo "export SERVING_BIN=$PWD/serving_bin/serving">>/root/.bashrc
    rm -rf serving-gpu-102-${SERVING_VERSION}.tar.gz
    cd -
elif [[ "$RUN_ENV" == "cuda11.2" ]];then
    python$PYTHON_VERSION -m pip install $client_release $app_release $server_release
    python$PYTHON_VERSION -m pip install $paddle_whl
    cd /usr/local/
    wget $serving_bin
    tar xf serving-gpu-112-${SERVING_VERSION}.tar.gz
    mv $PWD/serving-gpu-112-${SERVING_VERSION} $PWD/serving_bin
    echo "export SERVING_BIN=$PWD/serving_bin/serving">>/root/.bashrc
    rm -rf serving-gpu-112-${SERVING_VERSION}.tar.gz
    cd -
fi


