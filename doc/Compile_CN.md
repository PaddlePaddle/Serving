# 如何编译PaddleServing

(简体中文|[English](./Compile_EN.md))

## 总体概述

编译Paddle Serving一共分以下几步

- 编译环境准备：根据模型和运行环境的需要，选择最合适的镜像
- 下载代码库：下载Serving代码库，按需要执行初始化操作
- 环境变量准备：根据运行环境的需要，确定Python各个环境变量，如GPU环境还需要确定Cuda，Cudnn，TensorRT等环境变量。
- 正式编译： 编译`paddle-serving-server`, `paddle-serving-client`, `paddle-serving-app`相关whl包
- 安装相关whl包：安装编译出的三个whl包，并设置SERVING_BIN环境变量

此外，针对某些C++二次开发场景，我们也提供了OPENCV的联编方案。


## 编译环境准备

|             组件             |             版本要求              |
| :--------------------------: | :-------------------------------: |
|              OS              |     Ubuntu16 and 18/CentOS 7      |
|             gcc              |          5.4.0(Cuda 10.1) and 8.2.0         |
|           gcc-c++            |          5.4.0(Cuda 10.1) and 8.2.0         |
|            cmake             |          3.2.0 and later          |
|            Python            |          3.6.0 and later          |
|              Go              |          1.17.2 and later          |
|             git              |         2.17.1 and later          |
|         glibc-static         |               2.17                |
|        openssl-devel         |              1.0.2k               |
|         bzip2-devel          |          1.0.6 and later          |
| python-devel / python3-devel |          3.6.0 and later          |
|         sqlite-devel         |         3.7.17 and later          |
|           patchelf           |                0.9                |
|           libXext            |               1.3.3               |
|            libSM             |               1.2.2               |
|          libXrender          |              0.9.10               |

推荐使用Docker编译，我们已经为您准备好了Paddle Serving编译环境并配置好了上述编译依赖，详见[该文档](Docker_Images_CN.md)。

我们提供了五个环境的开发镜像，分别是CPU、 CUDA10.1+CUDNN7、CUDA10.2+CUDNN8、 CUDA11.2+CUDNN8。我们提供了Serving开发镜像涵盖以上环境。与此同时，我们也支持Paddle开发镜像。

Serving开发镜像是Serving套件为了支持各个预测环境提供的用于编译、调试预测服务的镜像，Paddle开发镜像是Paddle在官网发布的用于编译、开发、训练模型使用镜像。为了让Paddle开发者能够在同一个容器内直接使用Serving。对于上个版本就已经使用Serving用户的开发者来说，Serving开发镜像应该不会感到陌生。但对于熟悉Paddle训练框架生态的开发者，目前应该更熟悉已有的Paddle开发镜像。为了适应所有用户的不同习惯，我们对这两套镜像都做了充分的支持。


|  环境                         |   Serving开发镜像Tag               |    操作系统      | Paddle开发镜像Tag       |  操作系统            |
| :--------------------------: | :-------------------------------: | :-------------: | :-------------------: | :----------------: |
|  CPU                         | 0.9.0-devel                       |  Ubuntu 16.04   | 2.3.0                 | Ubuntu 18.04.       |
|  CUDA10.1 + CUDNN7             | 0.9.0-cuda10.1-cudnn7-devel       |  Ubuntu 16.04   | 无                     | 无                 |
|  CUDA10.2 + CUDNN8             | 0.9.0-cuda10.2-cudnn8-devel       |  Ubuntu 16.04   | 无                    |  无                 |
|  CUDA11.2 + CUDNN8             | 0.9.0-cuda11.2-cudnn8-devel       |  Ubuntu 16.04   | 2.3.0-gpu-cuda11.2-cudnn8 | Ubuntu 18.04        | 

我们首先要针对自己所需的环境拉取相关镜像。上表**环境**一列下，除了CPU，其余（Cuda**+Cudnn**）都属于GPU环境。
您可以使用Serving开发镜像。
```
docker pull registry.baidubce.com/paddlepaddle/serving:${Serving开发镜像Tag}

# 如果是GPU镜像
nvidia-docker run --rm -it registry.baidubce.com/paddlepaddle/serving:${Serving开发镜像Tag} bash

# 如果是CPU镜像
docker run --rm -it registry.baidubce.com/paddlepaddle/serving:${Serving开发镜像Tag} bash
```

也可以使用Paddle开发镜像。
```
docker pull registry.baidubce.com/paddlepaddle/paddle:${Paddle开发镜像Tag}

# 如果是GPU镜像，需要使用nvidia-docker
nvidia-docker run --rm -it registry.baidubce.com/paddlepaddle/paddle:${Paddle开发镜像Tag} bash

# 如果是CPU镜像
docker run --rm -it registry.baidubce.com/paddlepaddle/paddle:${Paddle开发镜像Tag} bash
```


## 下载代码库
**注明： 如果您正在使用Paddle开发镜像，需要在下载代码库后手动运行`bash env_install.sh`(如代码框的第三行所示）**
```
git clone https://github.com/PaddlePaddle/Serving
cd Serving && git submodule update --init --recursive

# Paddle开发镜像需要运行如下命令，Serving开发镜像不需要运行
bash tools/paddle_env_install.sh
```

## 环境变量准备

**设置PYTHON环境变量**

如果您使用的是Serving开发镜像，请按照如下，确定好需要编译的Python版本，设置对应的环境变量，一共需要设置三个环境变量，分别是`PYTHON_INCLUDE_DIR`, `PYTHON_LIBRARIES`, `PYTHON_EXECUTABLE`。以下我们以python 3.7为例，介绍如何设置这三个环境变量。

1) 设置`PYTHON_INCLUDE_DIR`

搜索Python.h 所在的目录
```
find / -name Python.h
```
通常会有类似于`**/include/python3.7/Python.h`出现，我们只需要取它的文件夹目录就好，比如找到`/usr/include/python3.7/Python.h`，那么我们只需要`export PYTHON_INCLUDE_DIR=/usr/include/python3.7/`就好。
如果没有找到。说明 1）没有安装开发版本的Python，需重新安装 2）权限不足无法查看相关系统目录。

2) 设置`PYTHON_LIBRARIES`

搜索 libpython3.7.so 或 libpython3.7m.so
```
find / -name libpython3.7.so
find / -name libpython3.7m.so
```
通常会有类似于`**/lib/libpython3.7.so`或者`**/lib/x86_64-linux-gnu/libpython3.7.so`出现，我们只需要取它的文件夹目录就好，比如找到`/usr/local/lib/libpython3.7.so`，那么我们只需要`export PYTHON_LIBRARIES=/usr/local/lib`就好。
如果没有找到，说明 1）静态编译Python，需要重新安装动态编译的Python 2）全县不足无法查看相关系统目录。

3) 设置`PYTHON_EXECUTABLE`

直接查看python3.7路径
```
which python3.7
```
假如结果是`/usr/local/bin/python3.7`，那么直接设置`export PYTHON_EXECUTABLE=/usr/local/bin/python3.7`。

设置好这三个环境变量至关重要，设置完成后，我们便可以执行下列操作（以下是Paddle Cuda 11.2的开发镜像的PYTHON环境，如果是其他镜像，请更改相应的`PYTHON_INCLUDE_DIR`, `PYTHON_LIBRARIES`, `PYTHON_EXECUTABLE`）。

```
# 以下三个环境变量是Paddle开发镜像Cuda11.2的环境，如其他镜像可能需要修改
export PYTHON_INCLUDE_DIR=/usr/include/python3.7m/
export PYTHON_LIBRARIES=/usr/lib/x86_64-linux-gnu/libpython3.7m.so
export PYTHON_EXECUTABLE=/usr/bin/python3.7

export GOPATH=$HOME/go
export PATH=$PATH:$GOPATH/bin

python3.7 -m pip install -r python/requirements.txt
 
go env -w GO111MODULE=on
go env -w GOPROXY=https://goproxy.cn,direct
go install github.com/grpc-ecosystem/grpc-gateway/protoc-gen-grpc-gateway@v1.15.2
go install github.com/grpc-ecosystem/grpc-gateway/protoc-gen-swagger@v1.15.2
go install github.com/golang/protobuf/protoc-gen-go@v1.4.3
go install google.golang.org/grpc@v1.33.0
go env -w GO111MODULE=auto
```

如果您是GPU用户需要额外设置`CUDA_PATH`, `CUDNN_LIBRARY`, `CUDA_CUDART_LIBRARY`和`TENSORRT_LIBRARY_PATH`。
```
export CUDA_PATH='/usr/local/cuda'
export CUDNN_LIBRARY='/usr/local/cuda/lib64/'
export CUDA_CUDART_LIBRARY="/usr/local/cuda/lib64/"
export TENSORRT_LIBRARY_PATH="/usr/"
```
环境变量的含义如下表所示。

| cmake环境变量         | 含义                                | GPU环境注意事项               | Docker环境是否需要 |
|-----------------------|-------------------------------------|-------------------------------|--------------------|
| CUDA_TOOLKIT_ROOT_DIR | cuda安装路径，通常为/usr/local/cuda | 全部GPU环境都需要                | 否(/usr/local/cuda)                 |
| CUDNN_LIBRARY         | libcudnn.so.*所在目录，通常为/usr/local/cuda/lib64/  | 全部GPU环境都需要                | 否(/usr/local/cuda/lib64/)                 |
| CUDA_CUDART_LIBRARY   | libcudart.so.*所在目录，通常为/usr/local/cuda/lib64/ | 全部GPU环境都需要                | 否(/usr/local/cuda/lib64/)                 |
| TENSORRT_ROOT         | libnvinfer.so.*所在目录的上一级目录，取决于TensorRT安装目录 | 全部GPU环境都需要 | 否(/usr)                 |



## 正式编译

我们一共需要编译三个目标，分别是`paddle-serving-server`, `paddle-serving-client`, `paddle-serving-app`，其中`paddle-serving-server`需要区分CPU或者GPU版本。如果是CPU版本请运行，

### 编译paddle-serving-server

```
mkdir build_server
cd build_server
cmake -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR \
    -DPYTHON_LIBRARIES=$PYTHON_LIBRARIES \
    -DPYTHON_EXECUTABLE=$PYTHON_EXECUTABLE \
    -DSERVER=ON \
    -DWITH_GPU=OFF ..
make -j20
cd ..
```

如果是GPU版本，请运行，
```
mkdir build_server
cd build_server
cmake -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR \
    -DPYTHON_LIBRARIES=$PYTHON_LIBRARIES \
    -DPYTHON_EXECUTABLE=$PYTHON_EXECUTABLE \
    -DCUDA_TOOLKIT_ROOT_DIR=${CUDA_PATH} \
    -DCUDNN_LIBRARY=${CUDNN_LIBRARY} \
    -DCUDA_CUDART_LIBRARY=${CUDA_CUDART_LIBRARY} \
    -DTENSORRT_ROOT=${TENSORRT_LIBRARY_PATH} \
    -DSERVER=ON \
    -DWITH_GPU=ON ..
make -j20
cd ..
``` 

### 编译paddle-serving-client 和 paddle-serving-app

接下来，我们继续编译client和app就可以了，这两个包的编译命令在所有平台通用，不区分CPU和GPU的版本。
```
# 编译paddle-serving-client
mkdir build_client
cd build_client
cmake -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR \
    -DPYTHON_LIBRARIES=$PYTHON_LIBRARIES \
    -DPYTHON_EXECUTABLE=$PYTHON_EXECUTABLE \
    -DCLIENT=ON ..
make -j10
cd ..

# 编译paddle-serving-app
mkdir build_app
cd build_app
cmake -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR \
    -DPYTHON_LIBRARIES=$PYTHON_LIBRARIES \
    -DPYTHON_EXECUTABLE=$PYTHON_EXECUTABLE \
    -DAPP=ON ..
make -j10
cd ..
```

## 安装相关whl包
```
pip3.7 install build_server/python/dist/*.whl
pip3.7 install build_client/python/dist/*.whl
pip3.7 install build_app/python/dist/*.whl
export SERVING_BIN=${PWD}/build_server/core/general-server/serving
```

## 注意事项

注意到上一小节的最后一行`export SERVING_BIN`，运行python端Server时，会检查`SERVING_BIN`环境变量，如果想使用自己编译的二进制文件，请将设置该环境变量为对应二进制文件的路径，通常是`export SERVING_BIN=${BUILD_DIR}/core/general-server/serving`。
其中BUILD_DIR为`build_server`的绝对路径。
可以cd build_server路径下，执行`export SERVING_BIN=${PWD}/core/general-server/serving`


## 开启WITH_OPENCV选项编译C++ Server

**注意：** 只有当您需要对Paddle Serving C++部分进行二次开发，且新增的代码依赖于OpenCV库时，您才需要这样做。

编译Serving C++ Server部分，开启WITH_OPENCV选项时，需要已安装的OpenCV库，若尚未安装，可参考本文档后面的说明编译安装OpenCV库。

以开启WITH_OPENCV选项，编译CPU版本Paddle Inference Library为例，在上述编译命令基础上，加入`DOPENCV_DIR=${OPENCV_DIR}` 和 `DWITH_OPENCV=ON`选项。
``` shell
OPENCV_DIR=your_opencv_dir #`your_opencv_dir`为opencv库的安装路径。
mkdir build_server && cd build_server
cmake -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR/ \
    -DPYTHON_LIBRARIES=$PYTHON_LIBRARIES \
    -DPYTHON_EXECUTABLE=$PYTHON_EXECUTABLE \
    -DOPENCV_DIR=${OPENCV_DIR} \
    -DWITH_OPENCV=ON \
    -DSERVER=ON ..
make -j10
```

**注意：** 编译成功后，需要设置`SERVING_BIN`路径。





## 附：CMake选项说明

|     编译选项     |                    说明                    | 默认 |
| :--------------: | :----------------------------------------: | :--: |
|     WITH_AVX     | Compile Paddle Serving with AVX intrinsics | OFF  |
|     WITH_MKL     |  Compile Paddle Serving with MKL support   | OFF  |
|     WITH_GPU     |   Compile Paddle Serving with NVIDIA GPU   | OFF  |
|     WITH_TRT     |    Compile Paddle Serving with TensorRT    | OFF  |
|     WITH_OPENCV  |    Compile Paddle Serving with OPENCV      | OFF  |
|  CUDNN_LIBRARY   |    Define CuDNN library and header path    |      |
| CUDA_TOOLKIT_ROOT_DIR |       Define CUDA PATH                |      |
|   TENSORRT_ROOT  |           Define TensorRT PATH             |      |
|      CLIENT      |       Compile Paddle Serving Client        | OFF  |
|      SERVER      |       Compile Paddle Serving Server        | OFF  |
|       APP        |     Compile Paddle Serving App package     | OFF  |
|       PACK       |              Compile for whl               | OFF  |

### WITH_GPU选项

Paddle Serving通过PaddlePaddle预测库支持在GPU上做预测。WITH_GPU选项用于检测系统上CUDA/CUDNN等基础库，如检测到合适版本，在编译PaddlePaddle时就会编译出GPU版本的OP Kernel。

在裸机上编译Paddle Serving GPU版本，需要安装这些基础库：

- CUDA
- CuDNN

编译TensorRT版本，需要安装TensorRT库。

这里要注意的是：

1. 编译Serving所在的系统上所安装的CUDA/CUDNN等基础库版本，需要兼容实际的GPU设备。例如，Tesla V100卡至少要CUDA 9.0。如果编译时所用CUDA等基础库版本过低，由于生成的GPU代码和实际硬件设备不兼容，会导致Serving进程无法启动，或出现coredump等严重问题。
2. 运行Paddle Serving的系统上安装与实际GPU设备兼容的CUDA driver，并安装与编译期所用的CUDA/CuDNN等版本兼容的基础库。如运行Paddle Serving的系统上安装的CUDA/CuDNN的版本低于编译时所用版本，可能会导致奇怪的cuda函数调用失败等问题。

以下是PaddleServing 镜像的Cuda与Cudnn，TensorRT的匹配关系，供参考：

|          |  CUDA   |   CuDNN      | TensorRT |
| :----:   | :-----: | :----------: | :----:   |
| post101  |  10.1   | CuDNN 7.6.5  | 6.0.1    |
| post102  |  10.2   | CuDNN 8.0.5  | 7.1.3    |
| post11   |  11.0   | CuDNN 8.0.4  | 7.1.3    |

### 附：如何让Paddle Serving编译系统探测到CuDNN库

从NVIDIA developer官网下载对应版本CuDNN并在本地解压后，在cmake编译命令中增加`-DCUDNN_LIBRARY`参数，指定CuDNN库所在路径。

## 附：编译安装OpenCV库
**注意：** 只有当您需要在C++代码中引入OpenCV库时，您才需要这样做。

* 首先需要从OpenCV官网上下载在Linux环境下源码编译的包，以OpenCV3.4.7为例，下载命令如下。

```
wget https://github.com/opencv/opencv/archive/3.4.7.tar.gz
tar -xf 3.4.7.tar.gz
```

最终可以在当前目录下看到`opencv-3.4.7/`的文件夹。

* 编译OpenCV，设置OpenCV源码路径(`root_path`)以及安装路径(`install_path`)。进入OpenCV源码路径下，按照下面的方式进行编译。

```shell
root_path=your_opencv_root_path
install_path=${root_path}/opencv3

rm -rf build
mkdir build
cd build

cmake .. \
    -DCMAKE_INSTALL_PREFIX=${install_path} \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DWITH_IPP=OFF \
    -DBUILD_IPP_IW=OFF \
    -DWITH_LAPACK=OFF \
    -DWITH_EIGEN=OFF \
    -DCMAKE_INSTALL_LIBDIR=lib64 \
    -DWITH_ZLIB=ON \
    -DBUILD_ZLIB=ON \
    -DWITH_JPEG=ON \
    -DBUILD_JPEG=ON \
    -DWITH_PNG=ON \
    -DBUILD_PNG=ON \
    -DWITH_TIFF=ON \
    -DBUILD_TIFF=ON

make -j
make install
```


其中`root_path`为下载的OpenCV源码路径，`install_path`为OpenCV的安装路径，`make install`完成之后，会在该文件夹下生成OpenCV头文件和库文件，用于引用OpenCV库的代码的编译。

最终在安装路径下的文件结构如下所示。

```
opencv3/
|-- bin
|-- include
|-- lib
|-- lib64
|-- share
```
