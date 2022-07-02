## Paddle Serving使用昇腾NPU芯片部署

Paddle Serving支持使用昇腾NPU芯片进行预测部署。目前支持在昇腾芯片(910/310)和arm服务器上进行部署，后续完善对其他异构硬件服务器部署能力。

## 昇腾910

### 安装Docker镜像
我们推荐使用docker部署Serving服务，可以直接从Paddle的官方镜像库拉取预先装有 CANN 社区版 5.0.2.alpha005 的 docker 镜像。
```
# 拉取镜像
docker pull paddlepaddle/paddle:latest-dev-cann5.0.2.alpha005-gcc82-aarch64

# 启动容器，注意这里的参数 --device，容器仅映射设备ID为4到7的4张NPU卡，如需映射其他卡相应增改设备ID号即可
docker run -it --name paddle-npu-dev -v /home/<user_name>:/workspace  \
            --pids-limit 409600 --network=host --shm-size=128G \
            --cap-add=SYS_PTRACE --security-opt seccomp=unconfined \
            --device=/dev/davinci4 --device=/dev/davinci5 \
            --device=/dev/davinci6 --device=/dev/davinci7 \
            --device=/dev/davinci_manager \
            --device=/dev/devmm_svm \
            --device=/dev/hisi_hdc \
            -v /usr/local/Ascend/driver:/usr/local/Ascend/driver \
            -v /usr/local/bin/npu-smi:/usr/local/bin/npu-smi \
            -v /usr/local/dcmi:/usr/local/dcmi \
            paddlepaddle/paddle:latest-dev-cann5.0.2.alpha005-gcc82-aarch64 /bin/bash

# 检查容器中是否可以正确识别映射的昇腾DCU设备
npu-smi info

# 预期得到类似如下的结果
+------------------------------------------------------------------------------------+
| npu-smi 1.9.3                    Version: 21.0.rc1                                 |
+----------------------+---------------+---------------------------------------------+
| NPU   Name           | Health        | Power(W)   Temp(C)                          |
| Chip                 | Bus-Id        | AICore(%)  Memory-Usage(MB)  HBM-Usage(MB)  |
+======================+===============+=============================================+
| 4     910A           | OK            | 67.2       30                               |
| 0                    | 0000:C2:00.0  | 0          303  / 15171      0    / 32768   |
+======================+===============+=============================================+
| 5     910A           | OK            | 63.8       25                               |
| 0                    | 0000:82:00.0  | 0          2123 / 15171      0    / 32768   |
+======================+===============+=============================================+
| 6     910A           | OK            | 67.1       27                               |
| 0                    | 0000:42:00.0  | 0          1061 / 15171      0    / 32768   |
+======================+===============+=============================================+
| 7     910A           | OK            | 65.5       30                               |
| 0                    | 0000:02:00.0  | 0          2563 / 15078      0    / 32768   |
+======================+===============+=============================================+
```

### 编译、安装
基本环境配置可参考[该文档](Compile_CN.md)进行配置。

***1、依赖安装***

安装编译所需依赖库，包括patchelf、libcurl等
```
apt-get install patchelf libcurl4-openssl-dev libbz2-dev libgeos-dev
```

***2、GOLANG环境配置***

下载并配置ARM版本的GOLANG-1.17.2
```
wget https://golang.org/dl/go1.17.2.linux-arm64.tar.gz
tar zxvf go1.17.2.linux-arm64.tar.gz -C /usr/local/
mkdir /root/go /root/go/bin /root/go/src
echo "GOROOT=/usr/local/go" >> /root/.bashrc
echo "GOPATH=/root/go" >> /root/.bashrc
echo "PATH=/usr/local/go/bin:/root/go/bin:$PATH" >> /root/.bashrc
source /root/.bashrc

go env -w GO111MODULE=on
go env -w GOPROXY=https://goproxy.cn,direct
go install github.com/grpc-ecosystem/grpc-gateway/protoc-gen-grpc-gateway@v1.15.2
go install github.com/grpc-ecosystem/grpc-gateway/protoc-gen-swagger@v1.15.2
go install github.com/golang/protobuf/protoc-gen-go@v1.4.3
go install google.golang.org/grpc@v1.33.1
go env -w GO111MODULE=auto
```

***3、PYTHON环境配置***

下载python依赖库并配置环境
```
pip3.7 install -r python/requirements.txt -i https://mirror.baidu.com/pypi/simple
export PYTHONROOT=/opt/conda
export PYTHON_INCLUDE_DIR=$PYTHONROOT/include/python3.7m
export PYTHON_LIBRARIES=$PYTHONROOT/lib/libpython3.7m.so
export PYTHON_EXECUTABLE=$PYTHONROOT/bin/python3.7
```

***4、编译server***

```
mkdir build-server-npu && cd build-server-npu
cmake -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR/ \
    -DPYTHON_LIBRARIES=$PYTHON_LIBRARIES \
    -DPYTHON_EXECUTABLE=$PYTHON_EXECUTABLE \
    -DCMAKE_INSTALL_PREFIX=./output \
    -DWITH_ASCEND_CL=ON \
    -DSERVER=ON ..
make TARGET=ARMV8 -j16
```

***5、安装编译包***

编译步骤完成后，会在各自编译目录$build_dir/python/dist生成whl包，分别安装即可。例如server步骤，会在server-build-npu/python/dist目录下生成whl包, 使用命令```pip install -u xxx.whl```进行安装。

### 部署使用
为了支持arm+昇腾910服务部署，启动服务时需使用以下参数。
| 参数     | 参数说明                    | 备注                                                             |
| :------- | :-------------------------- | :--------------------------------------------------------------- |
| use_ascend_cl | 使用Ascend CL进行预测      | 使用Ascend预测能力                                      |

以[Bert](../examples/C++/PaddleNLP/bert/README_CN.md)为例

启动rpc服务，使用Ascend npu优化加速能力
```
python3 -m paddle_serving_server.serve --model bert_seq128_model --thread 6 --port 9292 --use_ascend_cl
```

## 昇腾310

### 安装Docker镜像
我们推荐使用docker部署Serving服务，可以拉取装有 CANN 3.3.0 docker 镜像。
```
# 拉取镜像
docker pull registry.baidubce.com/paddlepaddle/serving:ascend-aarch64-cann3.3.0-paddlelite-devel

# 启动容器，注意这里的参数 --device，容器仅映射设备ID为4到7的4张NPU卡，如需映射其他卡相应增改设备ID号即可
docker run -it --name paddle-npu-dev -v /home/<user_name>:/workspace  \
            --pids-limit 409600 --network=host --shm-size=128G \
            --cap-add=SYS_PTRACE --security-opt seccomp=unconfined \
            --device=/dev/davinci4 --device=/dev/davinci5 \
            --device=/dev/davinci6 --device=/dev/davinci7 \
            --device=/dev/davinci_manager \
            --device=/dev/devmm_svm \
            --device=/dev/hisi_hdc \
            -v /usr/local/Ascend/driver:/usr/local/Ascend/driver \
            -v /usr/local/bin/npu-smi:/usr/local/bin/npu-smi \
            -v /usr/local/dcmi:/usr/local/dcmi \
            registry.baidubce.com/paddlepaddle/serving:ascend-aarch64-cann3.3.0-paddlelite-devel /bin/bash

```

### 编译、安装
基本环境配置可参考[该文档](Compile_CN.md)进行配置。

***1、PYTHON环境配置***

下载python依赖库并配置环境
```
pip3.7 install -r python/requirements.txt -i https://mirror.baidu.com/pypi/simple
export PYTHONROOT=/usr/local/python3.7.5
export PYTHON_INCLUDE_DIR=$PYTHONROOT/include/python3.7m
export PYTHON_LIBRARIES=$PYTHONROOT/lib/libpython3.7m.so
export PYTHON_EXECUTABLE=$PYTHONROOT/bin/python3.7
```

***2、编译server***

```
mkdir build-server-npu && cd build-server-npu
cmake -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR/ \
    -DPYTHON_LIBRARIES=$PYTHON_LIBRARIES \
    -DPYTHON_EXECUTABLE=$PYTHON_EXECUTABLE \
    -DCMAKE_INSTALL_PREFIX=./output \
    -DWITH_ASCEND_CL=ON \
    -DWITH_LITE=ON \
    -DSERVER=ON ..
make TARGET=ARMV8 -j16
```

***3、安装编译包***

编译步骤完成后，会在各自编译目录$build_dir/python/dist生成whl包，分别安装即可。例如server步骤，会在server-build-npu/python/dist目录下生成whl包, 使用命令```pip install -u xxx.whl```进行安装。

### 部署使用
为了支持arm+昇腾310服务部署，启动服务时需使用以下参数。
| 参数     | 参数说明                    | 备注                                                             |
| :------- | :-------------------------- | :--------------------------------------------------------------- |
| use_ascend_cl | 使用Ascend CL进行预测      | 使用Ascend预测能力                                      |
| use_lite | 使用Paddle-Lite Engine      | 使用Paddle-Lite cpu预测能力                                      |

以[resnet50](../examples/C++/PaddleClas/resnet_v2_50/README_CN.md)为例

启动rpc服务，使用Paddle-Lite npu优化加速能力
```
python3 -m paddle_serving_server.serve --model resnet_v2_50_imagenet_model --thread 6 --port 9292 --use_ascend_cl --use_lite
```

## 其他说明

### NPU芯片支持相关参考资料
* [昇腾NPU芯片运行飞桨](https://www.paddlepaddle.org.cn/documentation/docs/zh/guides/09_hardware_support/npu_docs/paddle_install_cn.html)
