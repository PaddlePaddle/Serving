# How to compile PaddleServing

([简体中文](./COMPILE_CN.md)|English)

## Compilation environment requirements

|            module            |              version              |
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

It is recommended to use Docker for compilation. We have prepared the Paddle Serving compilation environment for you, see [this document](DOCKER_IMAGES.md).

## Get Code

``` python
git clone https://github.com/PaddlePaddle/Serving
cd Serving && git submodule update --init --recursive
```

## PYTHONROOT settings

```shell
# For example, the path of python is /usr/bin/python, you can set PYTHONROOT
export PYTHONROOT=/usr
```

If you are using a Docker development image, please follow the following to determine the Python version to be compiled, and set the corresponding environment variables

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

## Install Python dependencies

```shell
pip install -r python/requirements.txt -i https://mirror.baidu.com/pypi/simple
```

If you use other Python version, please use the right `pip` accordingly.

## GOPATH Setting
The default GOPATH is set to `$HOME/go`, you can also set it to other values. **If it is the Docker environment provided by Serving, you do not need to set up.**

```shell
export GOPATH=$HOME/go
export PATH=$PATH:$GOPATH/bin
```

## Get go packages

```shell
go env -w GO111MODULE=on
go env -w GOPROXY=https://goproxy.cn,direct
go get -u github.com/grpc-ecosystem/grpc-gateway/protoc-gen-grpc-gateway@v1.15.2
go get -u github.com/grpc-ecosystem/grpc-gateway/protoc-gen-swagger@v1.15.2
go get -u github.com/golang/protobuf/protoc-gen-go@v1.4.3
go get -u google.golang.org/grpc@v1.33.0
```


## Compile Server

### Integrated CPU version paddle inference library

``` shell
mkdir server-build-cpu && cd server-build-cpu
cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ \
    -DPYTHON_LIBRARIES=$PYTHONROOT/lib/libpython2.7.so \
    -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python \
    -DSERVER=ON ..
make -j10
```

you can execute `make install` to put targets under directory `./output`, you need to add`-DCMAKE_INSTALL_PREFIX=./output`to specify output path to cmake command shown above.

### Integrated GPU version paddle inference library

Compared with CPU environment, GPU environment needs to refer to the following table,
**It should be noted that the following table is used as a reference for non-Docker compilation environment. The Docker compilation environment has been configured with relevant parameters and does not need to be specified in cmake process. **

| cmake environment variable | meaning | GPU environment considerations | whether Docker environment is needed |
|-----------------------|------------------------- ------------|-------------------------------|----- ---------------|
| CUDA_TOOLKIT_ROOT_DIR | cuda installation path, usually /usr/local/cuda | Required for all environments | No
(/usr/local/cuda) |
| CUDNN_LIBRARY | The directory where libcudnn.so.* is located, usually /usr/local/cuda/lib64/ | Required for all environments | No (/usr/local/cuda/lib64/) |
| CUDA_CUDART_LIBRARY | The directory where libcudart.so.* is located, usually /usr/local/cuda/lib64/ | Required for all environments | No (/usr/local/cuda/lib64/) |
| TENSORRT_ROOT | The upper level directory of the directory where libnvinfer.so.* is located, depends on the TensorRT installation directory | Cuda 9.0/10.0 does not need, other needs | No (/usr) |

If not in Docker environment, users can refer to the following execution methods. The specific path is subject to the current environment, and the code is only for reference.

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
    -DTENSORRT_ROOT=${TENSORRT_LIBRARY_PATH}
    -DSERVER=ON \
    -DWITH_GPU=ON ..
make -j10
```

Execute `make install` to put the target output in the `./output` directory.

**Note:** After the compilation is successful, you need to set the `SERVING_BIN` path, see the following [Notes](COMPILE.md#Notes) ).

## Compile Client

``` shell
mkdir client-build && cd client-build
cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ \
      -DPYTHON_LIBRARIES=$PYTHONROOT/lib/libpython2.7.so \
      -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python \
      -DCLIENT=ON ..
make -j10
```

execute `make install` to put targets under directory `./output`



## Compile the App

```bash
mkdir app-build && cd app-build
cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ \
    -DPYTHON_LIBRARIES=$PYTHONROOT/lib/libpython2.7.so \
    -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python \
    -DAPP=ON ..
make
```



## Install wheel package

Regardless of the client, server or App part, after compiling, install the whl package in `python/dist/` in the temporary directory(`server-build-cpu`, `server-build-gpu`, `client-build`,`app-build`) of the compilation process.
for example：cd server-build-cpu/python/dist && pip install -U xxxxx.whl


## Note

When running the python server, it will check the `SERVING_BIN` environment variable. If you want to use your own compiled binary file, set the environment variable to the path of the corresponding binary file, usually`export SERVING_BIN=${BUILD_DIR}/core/general-server/serving`.
BUILD_DIR is the absolute path of server build CPU or server build GPU。
for example: cd server-build-cpu && export SERVING_BIN=${PWD}/core/general-server/serving



## Verify

Please use the example under `python/examples` to verify.



## CMake Option Description

| Compile Options  |                    Description             | Default |
| :--------------: | :----------------------------------------: | :--: |
|     WITH_AVX     | Compile Paddle Serving with AVX intrinsics | OFF  |
|     WITH_MKL     |  Compile Paddle Serving with MKL support   | OFF  |
|     WITH_GPU     |   Compile Paddle Serving with NVIDIA GPU   | OFF  |
|  CUDNN_LIBRARY   |    Define CuDNN library and header path    |      |
| CUDA_TOOLKIT_ROOT_DIR |       Define CUDA PATH                |      |
|   TENSORRT_ROOT  |           Define TensorRT PATH             |      |
|      CLIENT      |       Compile Paddle Serving Client        | OFF  |
|      SERVER      |       Compile Paddle Serving Server        | OFF  |
|       APP        |     Compile Paddle Serving App package     | OFF  |
|       PACK       |              Compile for whl               | OFF  |

### WITH_GPU Option

Paddle Serving supports prediction on the GPU through the PaddlePaddle inference library. The WITH_GPU option is used to detect basic libraries such as CUDA/CUDNN on the system. If an appropriate version is detected, the GPU Kernel will be compiled when PaddlePaddle is compiled.

To compile the Paddle Serving GPU version on bare metal, you need to install these basic libraries:

- CUDA
- CuDNN

To compile the TensorRT version, you need to install the TensorRT library.

Note here:

1. The basic library versions such as CUDA/CUDNN installed on the system where Serving is compiled, needs to be compatible with the actual GPU device. For example, the Tesla V100 card requires at least CUDA 9.0. If the version of the basic library such as CUDA used during compilation is too low, the generated GPU code is not compatible with the actual hardware device, which will cause the Serving process to fail to start or serious problems such as coredump.
2. Install the CUDA driver compatible with the actual GPU device on the system running Paddle Serving, and install the basic library compatible with the CUDA/CuDNN version used during compilation. If the version of CUDA/CuDNN installed on the system running Paddle Serving is lower than the version used at compile time, it may cause some cuda function call failures and other problems.


The following is the base library version matching relationship used by the PaddlePaddle release version for reference:

|          |  CUDA   |   CuDNN      | TensorRT |
| :----:   | :-----: | :----------: | :----:   |
| post9    |  9.0    | CuDNN 7.6.4  |          |
| post10   |  10.0   | CuDNN 7.6.5  |          |
| post101  |  10.1   | CuDNN 7.6.5  | 6.0.1    |
| post102  |  10.2   | CuDNN 8.0.5  | 7.1.3    |
| post11   |  11.0   | CuDNN 8.0.4  | 7.1.3    |

### How to make the compiler detect the CuDNN library

Download the corresponding CUDNN version from NVIDIA developer official website and decompressing it, add `-DCUDNN_ROOT` to cmake command, to specify the path of CUDNN.
