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

VERSION=$1
CUDNN=$2
if [[ "$VERSION" == "cuda10.1" ]];then
  wget -q https://paddle-ci.gz.bcebos.com/TRT/TensorRT6-cuda10.1-cudnn7.tar.gz --no-check-certificate
  tar -zxf TensorRT6-cuda10.1-cudnn7.tar.gz -C /usr/local
  cp -rf /usr/local/TensorRT6-cuda10.1-cudnn7/include/* /usr/include/ && cp -rf /usr/local/TensorRT6-cuda10.1-cudnn7/lib/* /usr/lib/
  echo "cuda10.1 trt install ==============>>>>>>>>>>>>"
  rm TensorRT6-cuda10.1-cudnn7.tar.gz
elif [[ "$VERSION" == "cuda11.2" ]];then
  wget https://paddle-ci.gz.bcebos.com/TRT/TensorRT-8.0.3.4.Linux.x86_64-gnu.cuda-11.3.cudnn8.2.tar.gz --no-check-certificate
  tar -zxf TensorRT-8.0.3.4.Linux.x86_64-gnu.cuda-11.3.cudnn8.2.tar.gz -C /usr/local
  cp -rf /usr/local/TensorRT-8.0.3.4/include/* /usr/include/ && cp -rf /usr/local/TensorRT-8.0.3.4/lib/* /usr/lib/
  rm -rf TensorRT-8.0.3.4.Linux.x86_64-gnu.cuda-11.3.cudnn8.2.tar.gz
elif [[ "$VERSION" == "cuda10.2" ]];then
  if [[ "$CUDNN" == "cudnn8" ]];then
    wget https://paddle-ci.gz.bcebos.com/TRT/TensorRT7-cuda10.2-cudnn8.tar.gz --no-check-certificate
    tar -zxf TensorRT7-cuda10.2-cudnn8.tar.gz -C /usr/local
    cp -rf /usr/local/TensorRT-7.1.3.4/include/* /usr/include/ && cp -rf /usr/local/TensorRT-7.1.3.4/lib/* /usr/lib/
    rm TensorRT7-cuda10.2-cudnn8.tar.gz
  elif [[ "$CUDNN" == "cudnn7" ]];then
    wget https://paddle-ci.gz.bcebos.com/TRT/TensorRT6-cuda10.2-cudnn7.tar.gz --no-check-certificate
    tar -zxf TensorRT6-cuda10.2-cudnn7.tar.gz -C /usr/local
    cp -rf /usr/local/TensorRT-6.0.1.8/include/*  /usr/include/ && cp -rf /usr/local/TensorRT-6.0.1.8/lib/* /usr/lib/
    rm -rf TensorRT6-cuda10.2-cudnn7.tar.gz
  fi
fi
