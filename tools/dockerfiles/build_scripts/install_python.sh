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

if [[ "$VERSION" == "2.7" ]];then
    wget -q https://www.python.org/ftp/python/2.7.15/Python-2.7.15.tgz && tar -xvf Python-2.7.15.tgz && cd Python-2.7.15 
    ./configure --enable-unicode=ucs4 --enable-shared CFLAGS=-fPIC --prefix=/usr/local/ && make && make install -j8 > /dev/null && make altinstall > /dev/null && ldconfig
    cd .. && rm -rf Python-2.7.15*
    wget https://bootstrap.pypa.io/pip/2.7/get-pip.py
    python2.7 get-pip.py
    rm -rf get-pip.py
elif [[ "$VERSION" == "3.6" ]];then
    wget -q https://www.python.org/ftp/python/3.6.8/Python-3.6.8.tgz && \
    tar -xzf Python-3.6.8.tgz && cd Python-3.6.8 && \
    CFLAGS="-Wformat" ./configure --prefix=/usr/local/ --enable-shared > /dev/null && \
    make -j8 > /dev/null && make altinstall > /dev/null && ldconfig
    cd .. && rm -rf Python-3.6.8*
    python3.6 -m pip install -U pip
elif [[ "$VERSION" == "3.7" ]];then
    wget -q https://www.python.org/ftp/python/3.7.0/Python-3.7.0.tgz && \
    tar -xzf Python-3.7.0.tgz && cd Python-3.7.0 && \
    CFLAGS="-Wformat" ./configure --prefix=/usr/local/ --enable-shared > /dev/null && \
    make -j8 > /dev/null && make altinstall > /dev/null && ldconfig
    cd .. && rm -rf Python-3.7.0*
    python3.7 -m pip install -U pip
elif [[ "$VERSION" == "3.8" ]];then
    wget -q https://www.python.org/ftp/python/3.8.0/Python-3.8.0.tgz && \
    tar -xzf Python-3.8.0.tgz && cd Python-3.8.0 && \
    CFLAGS="-Wformat" ./configure --prefix=/usr/local/ --enable-shared > /dev/null && \
    make -j8 > /dev/null && make altinstall > /dev/null && ldconfig
    cd .. && rm -rf Python-3.8.0*
    python3.8 -m pip install -U pip
fi


