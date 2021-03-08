# 如何编译PaddleServing

(简体中文|[English](./COMPILE.md))

## 编译环境设置

|             组件             |             版本要求              |
| :--------------------------: | :-------------------------------: |
|              OS              |     Ubuntu16 and 18/CentOS 7      |
|             gcc              | 4.8.5(Cuda 9.0 and 10.0) and 8.2(Others) |
|           gcc-c++            | 4.8.5(Cuda 9.0 and 10.0) and 8.2(Others) |
|            cmake             |          3.2.0 and later          |
|            Python            |  2.7.2 and later / 3.5.1 and later |
|              Go              |          1.9.2 and later          |
|             git              |         2.17.1 and later          |
|         glibc-static         |               2.17                |
|        openssl-devel         |              1.0.2k               |
|         bzip2-devel          |          1.0.6 and later          |
| python-devel / python3-devel | 2.7.5 and later / 3.6.8 and later |
|         sqlite-devel         |         3.7.17 and later          |
|           patchelf           |                0.9                |
|           libXext            |               1.3.3               |
|            libSM             |               1.2.2               |
|          libXrender          |              0.9.10               |

推荐使用Docker编译，我们已经为您准备好了Paddle Serving编译环境并配置好了上述编译依赖，详见[该文档](DOCKER_IMAGES_CN.md)。

## 获取代码

``` python
git clone https://github.com/PaddlePaddle/Serving
cd Serving && git submodule update --init --recursive
```

## PYTHONROOT设置

```shell
# 例如python的路径为/usr/bin/python，可以设置PYTHONROOT
export PYTHONROOT=/usr
```

如果您使用的是Docker开发镜像，请按照如下，确定好需要编译的Python版本，设置对应的环境变量
```
#Python 2.7
export PYTHONROOT=/usr/local/python2.7.15/
export PYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/
export PYTHON_LIBRARIES=$PYTHONROOT/lib/libpython2.7.so
export PYTHON_EXECUTABLE=$PYTHONROOT/bin/python2.7

#Python 3.5
export PYTHONROOT=/usr/local/python3.5.1
export PYTHON_INCLUDE_DIR=$PYTHONROOT/include/python3.5m
export PYTHON_LIBRARIES=$PYTHONROOT/lib/libpython3.5m.so
export PYTHON_EXECUTABLE=$PYTHONROOT/bin/python3.5

#Python3.6
export PYTHONROOT=/usr/local/
export PYTHON_INCLUDE_DIR=$PYTHONROOT/include/python3.6m
export PYTHON_LIBRARIES=$PYTHONROOT/lib/libpython3.6m.so
export PYTHON_EXECUTABLE=$PYTHONROOT/bin/python3.6

#Python3.7
export PYTHONROOT=/usr/local/
export PYTHON_INCLUDE_DIR=$PYTHONROOT/include/python3.7m
export PYTHON_LIBRARIES=$PYTHONROOT/lib/libpython3.7m.so
export PYTHON_EXECUTABLE=$PYTHONROOT/bin/python3.7

#Python3.8
export PYTHONROOT=/usr/local/
export PYTHON_INCLUDE_DIR=$PYTHONROOT/include/python3.8
export PYTHON_LIBRARIES=$PYTHONROOT/lib/libpython3.8.so
export PYTHON_EXECUTABLE=$PYTHONROOT/bin/python3.8

```

## 安装Python依赖

```shell
pip install -r python/requirements.txt -i https://mirror.baidu.com/pypi/simple
```

如果使用其他Python版本，请使用对应版本的`pip`。

## GOPATH 设置

默认 GOPATH 设置为 `$HOME/go`，您也可以设置为其他值。** 如果是Serving提供的Docker环境，可以不需要设置。**
```shell
export GOPATH=$HOME/go
export PATH=$PATH:$GOPATH/bin
```

## 获取 Go packages

```shell
go env -w GO111MODULE=on
go env -w GOPROXY=https://goproxy.cn,direct
go get -u github.com/grpc-ecosystem/grpc-gateway/protoc-gen-grpc-gateway@v1.15.2
go get -u github.com/grpc-ecosystem/grpc-gateway/protoc-gen-swagger@v1.15.2
go get -u github.com/golang/protobuf/protoc-gen-go@v1.4.3
go get -u google.golang.org/grpc@v1.33.0
```


## 编译Server部分

### 集成CPU版本Paddle Inference Library

``` shell
mkdir server-build-cpu && cd server-build-cpu
cmake -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR/ \
    -DPYTHON_LIBRARIES=$PYTHON_LIBRARIES \
    -DPYTHON_EXECUTABLE=$PYTHON_EXECUTABLE \
    -DSERVER=ON ..
make -j10
```

可以执行`make install`把目标产出放在`./output`目录下，cmake阶段需添加`-DCMAKE_INSTALL_PREFIX=./output`选项来指定存放路径。

### 集成GPU版本Paddle Inference Library

相比CPU环境，GPU环境需要参考以下表格,
**需要说明的是，以下表格对非Docker编译环境作为参考，Docker编译环境已经配置好相关参数，无需在cmake过程指定。**

| cmake环境变量         | 含义                                | GPU环境注意事项               | Docker环境是否需要 |
|-----------------------|-------------------------------------|-------------------------------|--------------------|
| CUDA_TOOLKIT_ROOT_DIR | cuda安装路径，通常为/usr/local/cuda | 全部环境都需要                | 否(/usr/local/cuda)                 |
| CUDNN_LIBRARY         | libcudnn.so.*所在目录，通常为/usr/local/cuda/lib64/  | 全部环境都需要                | 否(/usr/local/cuda/lib64/)                 |
| CUDA_CUDART_LIBRARY   | libcudart.so.*所在目录，通常为/usr/local/cuda/lib64/ | 全部环境都需要                | 否(/usr/local/cuda/lib64/)                 |
| TENSORRT_ROOT         | libnvinfer.so.*所在目录的上一级目录，取决于TensorRT安装目录 | Cuda 9.0/10.0不需要，其他需要 | 否(/usr)                 |

非Docker环境下，用户可以参考如下执行方式，具体的路径以当时环境为准，代码仅作为参考。TENSORRT_LIBRARY_PATH和TensorRT版本有关，要根据实际情况设置。例如在cuda10.1环境下TensorRT版本是6.0(/usr/local/TensorRT-6.0.1.5/targets/x86_64-linux-gnu/)，在cuda10.2环境下TensorRT版本是7.1（/usr/local/TensorRT-7.1.3.4/targets/x86_64-linux-gnu/）。

``` shell
export CUDA_PATH='/usr/local/cuda'
export CUDNN_LIBRARY='/usr/local/cuda/lib64/'
export CUDA_CUDART_LIBRARY="/usr/local/cuda/lib64/"
export TENSORRT_LIBRARY_PATH="/usr/local/TensorRT-6.0.1.5/targets/x86_64-linux-gnu/"

mkdir server-build-gpu && cd server-build-gpu
cmake -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR \
    -DPYTHON_LIBRARIES=$PYTHON_LIBRARIES \
    -DPYTHON_EXECUTABLE=$PYTHON_EXECUTABLE \
    -DCUDA_TOOLKIT_ROOT_DIR=${CUDA_PATH} \
    -DCUDNN_LIBRARY=${CUDNN_LIBRARY} \
    -DCUDA_CUDART_LIBRARY=${CUDA_CUDART_LIBRARY} \
    -DTENSORRT_ROOT=${TENSORRT_LIBRARY_PATH} \
    -DSERVER=ON \
    -DWITH_GPU=ON ..
make -j10
```

执行`make install`可以把目标产出放在`./output`目录下。

**注意：** 编译成功后，需要设置`SERVING_BIN`路径，详见后面的[注意事项](https://github.com/PaddlePaddle/Serving/blob/develop/doc/COMPILE_CN.md#注意事项)。

## 编译Client部分

``` shell
mkdir client-build && cd client-build
cmake -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR \
    -DPYTHON_LIBRARIES=$PYTHON_LIBRARIES \
    -DPYTHON_EXECUTABLE=$PYTHON_EXECUTABLE \
    -DCLIENT=ON ..
make -j10
```

执行`make install`可以把目标产出放在`./output`目录下。



## 编译App部分

```bash
mkdir app-build && cd app-build
cmake -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR \
    -DPYTHON_LIBRARIES=$PYTHON_LIBRARIES \
    -DPYTHON_EXECUTABLE=$PYTHON_EXECUTABLE \    
    -DAPP=ON ..
make
```



## 安装wheel包

无论是Client端，Server端还是App部分，编译完成后，安装编译过程临时目录（`server-build-cpu`、`server-build-gpu`、`client-build`、`app-build`）下的`python/dist/` 中的whl包即可。
例如：cd server-build-cpu/python/dist && pip install -U xxxxx.whl




## 注意事项

运行python端Server时，会检查`SERVING_BIN`环境变量，如果想使用自己编译的二进制文件，请将设置该环境变量为对应二进制文件的路径，通常是`export SERVING_BIN=${BUILD_DIR}/core/general-server/serving`。
其中BUILD_DIR为server-build-cpu或server-build-gpu的绝对路径。
可以cd server-build-cpu路径下，执行`export SERVING_BIN=${PWD}/core/general-server/serving`



## 如何验证

请使用 `python/examples` 下的例子进行验证。



## CMake选项说明

|     编译选项     |                    说明                    | 默认 |
| :--------------: | :----------------------------------------: | :--: |
|     WITH_AVX     | Compile Paddle Serving with AVX intrinsics | OFF  |
|     WITH_MKL     |  Compile Paddle Serving with MKL support   | OFF  |
|     WITH_GPU     |   Compile Paddle Serving with NVIDIA GPU   | OFF  |
|     WITH_TRT     |    Compile Paddle Serving with TensorRT    | OFF  |
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
| post9    |  9.0    | CuDNN 7.6.4  |          |
| post10   |  10.0   | CuDNN 7.6.5  |          |
| post101  |  10.1   | CuDNN 7.6.5  | 6.0.1    |
| post102  |  10.2   | CuDNN 8.0.5  | 7.1.3    |
| post11   |  11.0   | CuDNN 8.0.4  | 7.1.3    |

### 如何让Paddle Serving编译系统探测到CuDNN库

从NVIDIA developer官网下载对应版本CuDNN并在本地解压后，在cmake编译命令中增加`-DCUDNN_LIBRARY`参数，指定CuDNN库所在路径。
