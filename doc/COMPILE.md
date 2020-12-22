# How to compile PaddleServing

([简体中文](./COMPILE_CN.md)|English)

## Compilation environment requirements

|            module            |              version              |
| :--------------------------: | :-------------------------------: |
|              OS              |             CentOS 7              |
|             gcc              |          4.8.5 and later          |
|           gcc-c++            |          4.8.5 and later          |
|             git              |          3.82 and later           |
|            cmake             |          3.2.0 and later          |
|            Python            |  2.7.2 and later / 3.6 and later  |
|              Go              |          1.9.2 and later          |
|             git              |         2.17.1 and later          |
|         glibc-static         |               2.17                |
|        openssl-devel         |              1.0.2k               |
|         bzip2-devel          |          1.0.6 and later          |
| python-devel / python3-devel | 2.7.5 and later / 3.6.8 and later |
|         sqlite-devel         |         3.7.17 and later          |
|           patchelf           |           0.9 and later           |
|           libXext            |               1.3.3               |
|            libSM             |               1.2.2               |
|          libXrender          |              0.9.10               |

It is recommended to use Docker for compilation. We have prepared the Paddle Serving compilation environment for you, see [this document](DOCKER_IMAGES.md).

This document will take Python2 as an example to show how to compile Paddle Serving. If you want to compile with Python3, just adjust the Python options of cmake:

- Set `DPYTHON_INCLUDE_DIR` to `$PYTHONROOT/include/python3.6m/`
- Set  `DPYTHON_LIBRARIES` to `$PYTHONROOT/lib64/libpython3.6.so`
- Set `DPYTHON_EXECUTABLE` to `$PYTHONROOT/bin/python3.6`

## Get Code

``` python
git clone https://github.com/PaddlePaddle/Serving
cd Serving && git submodule update --init --recursive
```




## PYTHONROOT Setting

```shell
# for example, the path of python is /usr/bin/python, you can set /usr as PYTHONROOT
export PYTHONROOT=/usr/
```

In the default centos7 image we provide, the Python path is `/usr/bin/python`. If you want to use our centos6 image, you need to set it to `export PYTHONROOT=/usr/local/python2.7/`.



## Install Python dependencies

```shell
pip install -r python/requirements.txt
```

If Python3 is used, replace `pip` with `pip3`.

## GOPATH Setting


## Compile Arguments

The default GOPATH is `$HOME/go`, which you can set to other values.
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
### CUDA_PATH is the cuda install path,use the command(whereis cuda) to check,it should be /usr/local/cuda.
### CUDNN_LIBRARY && CUDA_CUDART_LIBRARY is the lib path, it should be /usr/local/cuda/lib64/
 
``` shell
export CUDA_PATH='/usr/local'
export CUDNN_LIBRARY='/usr/local/cuda/lib64/'
export CUDA_CUDART_LIBRARY="/usr/local/cuda/lib64/"

mkdir server-build-gpu && cd server-build-gpu
cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ \
    -DPYTHON_LIBRARIES=$PYTHONROOT/lib/libpython2.7.so \
    -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python \
    -DCUDA_TOOLKIT_ROOT_DIR=${CUDA_PATH} \
    -DCUDNN_LIBRARY=${CUDNN_LIBRARY} \
    -DCUDA_CUDART_LIBRARY=${CUDA_CUDART_LIBRARY} \  
    -DSERVER=ON \
    -DWITH_GPU=ON ..
make -j10
```

### Integrated TRT version paddle inference library

```
export CUDA_PATH='/usr/local'
export CUDNN_LIBRARY='/usr/local/cuda/lib64/'
export CUDA_CUDART_LIBRARY="/usr/local/cuda/lib64/"

mkdir server-build-trt && cd server-build-trt
cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ \
    -DPYTHON_LIBRARIES=$PYTHONROOT/lib/libpython2.7.so \
    -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python \
    -DTENSORRT_ROOT=${TENSORRT_LIBRARY_PATH} \
    -DCUDA_TOOLKIT_ROOT_DIR=${CUDA_PATH} \
    -DCUDNN_LIBRARY=${CUDNN_LIBRARY} \
    -DCUDA_CUDART_LIBRARY=${CUDA_CUDART_LIBRARY} \
    -DSERVER=ON \
    -DWITH_GPU=ON \
    -DWITH_TRT=ON ..
make -j10
```

execute `make install` to put targets under directory `./output`

**Attention：** After the compilation is successful, you need to set the path of `SERVING_BIN`. See [Note](https://github.com/PaddlePaddle/Serving/blob/develop/doc/COMPILE.md#Note) for details.



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
| WITH_ELASTIC_CTR |        Compile ELASITC-CTR solution        | OFF  |
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

|          |  CUDA   |          CuDNN           | TensorRT |
| :----:   | :-----: | :----------------------: | :----:   |
| post9    |  9.0    | CuDNN 7.3.1 for CUDA 9.0 |          |
| post10   |  10.0   | CuDNN 7.5.1 for CUDA 10.0|          |
| trt      |  10.1   | CuDNN 7.5.1 for CUDA 10.1| 6.0.1.5  |

### How to make the compiler detect the CuDNN library

Download the corresponding CUDNN version from NVIDIA developer official website and decompressing it, add `-DCUDNN_ROOT` to cmake command, to specify the path of CUDNN.
