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

client_release="paddle-serving-client==$SERVING_VERSION"
app_release="paddle-serving-app==0.3.1"
if [[ "$RUN_ENV" == "cpu" ]];then
    server_release="paddle-serving-server==$SERVING_VERSION"
    python$PYTHON_VERSION -m pip install $client_release $app_release $server_release
    python$PYTHON_VERSION -m pip install paddlepaddle==${PADDLE_VERSION}
    cd /usr/local/
    wget https://paddle-serving.bj.bcebos.com/bin/serving-cpu-noavx-openblas-${SERVING_VERSION}.tar.gz 
    tar xf serving-cpu-noavx-openblas-${SERVING_VERSION}.tar.gz
    echo "export SERVING_BIN=$PWD/serving-cpu-noavx-openblas-${SERVING_VERSION}/serving">>/root/.bashrc
    rm -rf serving-cpu-noavx-openblas-${SERVING_VERSION}.tar.gz
    cd -
elif [[ "$RUN_ENV" == "cuda10.1" ]];then
    server_release="paddle-serving-server-gpu==$SERVING_VERSION.post101"
    python$PYTHON_VERSION -m pip install $client_release $app_release $server_release
    python$PYTHON_VERSION -m pip install paddlepaddle-gpu==${PADDLE_VERSION}
    cd /usr/local/
    wget https://paddle-serving.bj.bcebos.com/bin/serving-gpu-101-${SERVING_VERSION}.tar.gz
    tar xf serving-gpu-101-${SERVING_VERSION}.tar.gz
    echo "export SERVING_BIN=$PWD/serving-gpu-101-${SERVING_VERSION}/serving">>/root/.bashrc
    rm -rf serving-gpu-101-${SERVING_VERSION}.tar.gz
    cd -
elif [[ "$RUN_ENV" == "cuda10.2" ]];then
    server_release="paddle-serving-server-gpu==$SERVING_VERSION.post102"
    python$PYTHON_VERSION -m pip install $client_release $app_release $server_release
    python$PYTHON_VERSION -m pip install paddlepaddle-gpu==${PADDLE_VERSION}
    cd /usr/local/
    wget https://paddle-serving.bj.bcebos.com/bin/serving-gpu-102-${SERVING_VERSION}.tar.gz
    tar xf serving-gpu-102-${SERVING_VERSION}.tar.gz
    echo "export SERVING_BIN=$PWD/serving-gpu-102-${SERVING_VERSION}/serving">>/root/.bashrc
    rm -rf serving-gpu-102-${SERVING_VERSION}.tar.gz
    cd -
fi


