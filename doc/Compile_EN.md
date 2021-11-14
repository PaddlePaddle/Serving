# How to compile PaddleServing

([简体中文](./Compile_CN.md)|English)

## Compilation environment requirements

|            module            |              version              |
| :--------------------------: | :-------------------------------: |
|              OS              |     Ubuntu16 and 18/CentOS 7      |
|             gcc              |          5.4.0(Cuda 10.1) and 8.2.0         |
|           gcc-c++            |          5.4.0(Cuda 10.1) and 8.2.0         |
|            cmake             |          3.2.0 and later          |
|            Python            |          3.6.0 and later          |
|              Go              |          1.9.2 and later          |
|             git              |         2.17.1 and later          |
|         glibc-static         |               2.17                |
|        openssl-devel         |              1.0.2k               |
|         bzip2-devel          |          1.0.6 and later          |
|        python3-devel         |         3.6.0 and later |
|         sqlite-devel         |         3.7.17 and later          |
|           patchelf           |                0.9                |
|           libXext            |               1.3.3               |
|            libSM             |               1.2.2               |
|          libXrender          |              0.9.10               |

It is recommended to use Docker for compilation. We have prepared the Paddle Serving compilation environment for you, see [this document](Docker_Images_EN.md).

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
go env -w GO111MODULE=auto
```


## Compile Server

### Integrated CPU version paddle inference library

``` shell
mkdir server-build-cpu && cd server-build-cpu
cmake -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR \
    -DPYTHON_LIBRARIES=$PYTHON_LIBRARIES \
    -DPYTHON_EXECUTABLE=$PYTHON_EXECUTABLE \
    -DSERVER=ON ..
make -j10
```

you can execute `make install` to put targets under directory `./output`, you need to add`-DCMAKE_INSTALL_PREFIX=./output`to specify output path to cmake command shown above.

### Integrated GPU version paddle inference library

Compared with CPU environment, GPU environment needs to refer to the following table,
**It should be noted that the following table is used as a reference for non-Docker compilation environment. The Docker compilation environment has been configured with relevant parameters and does not need to be specified in cmake process. **

| cmake environment variable | meaning | GPU environment considerations | whether Docker environment is needed |
|-----------------------|-------------------------------------|-------------------------------|--------------------|
| CUDA_TOOLKIT_ROOT_DIR | cuda installation path, usually /usr/local/cuda | Required for all environments | No (/usr/local/cuda) |
| CUDNN_LIBRARY | The directory where libcudnn.so.* is located, usually /usr/local/cuda/lib64/ | Required for all environments | No (/usr/local/cuda/lib64/) |
| CUDA_CUDART_LIBRARY | The directory where libcudart.so.* is located, usually /usr/local/cuda/lib64/ | Required for all environments | No (/usr/local/cuda/lib64/) |
| TENSORRT_ROOT | The upper level directory of the directory where libnvinfer.so.* is located, depends on the TensorRT installation directory | Cuda 9.0/10.0 does not need, other needs | No (/usr) |

If not in Docker environment, users can refer to the following execution methods. The specific path is subject to the current environment, and the code is only for reference.TENSORRT_LIBRARY_PATH is related to the TensorRT version and should be set according to the actual situation。For example, in the cuda10.1 environment, the TensorRT version is 6.0 (/usr/local/TensorRT6-cuda10.1-cudnn7/targets/x86_64-linux-gnu/)，In the cuda10.2 and cuda11.0 environment, the TensorRT version is 7.1 (/usr/local/TensorRT-7.1.3.4/targets/x86_64-linux-gnu/).

``` shell
export CUDA_PATH='/usr/local/cuda'
export CUDNN_LIBRARY='/usr/local/cuda/lib64/'
export CUDA_CUDART_LIBRARY="/usr/local/cuda/lib64/"

export TENSORRT_LIBRARY_PATH="/usr/local/TensorRT6-cuda10.1-cudnn7/targets/x86_64-linux-gnu/"

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

Execute `make install` to put the target output in the `./output` directory.

### Compile C++ Server under the condition of WITH_OPENCV=ON
**Note:** Only when you need to redevelop the paddle serving C + + part, and the new code depends on the OpenCV library, you need to do so.

First of all , OpenCV library should be installed, if not, please refer to the `Compile and install OpenCV` section later in this article.

In the compile command, add `DOPENCV_DIR=${OPENCV_DIR}` and `DWITH_OPENCV=ON`，for example：
``` shell
OPENCV_DIR=your_opencv_dir #`your_opencv_dir` is the installation path of OpenCV library。
mkdir server-build-cpu && cd server-build-cpu
cmake -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR/ \
    -DPYTHON_LIBRARIES=$PYTHON_LIBRARIES \
    -DPYTHON_EXECUTABLE=$PYTHON_EXECUTABLE \
    -DOPENCV_DIR=${OPENCV_DIR} \
    -DWITH_OPENCV=ON \
    -DSERVER=ON ..
make -j10
```
**Note:** After the compilation is successful, you need to set the `SERVING_BIN` path, see the following [Notes](Compile_EN.md#Notes).

## Compile Client

``` shell
mkdir client-build && cd client-build
cmake -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR \
    -DPYTHON_LIBRARIES=$PYTHON_LIBRARIES \
    -DPYTHON_EXECUTABLE=$PYTHON_EXECUTABLE \
    -DCLIENT=ON ..
make -j10
```

execute `make install` to put targets under directory `./output`



## Compile the App

```bash
mkdir app-build && cd app-build
cmake -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR \
    -DPYTHON_LIBRARIES=$PYTHON_LIBRARIES \
    -DPYTHON_EXECUTABLE=$PYTHON_EXECUTABLE \
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
|     WITH_OPENCV  |    Compile Paddle Serving with OPENCV      | OFF  |
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
| post101  |  10.1   | CuDNN 7.6.5  | 6.0.1    |
| post102  |  10.2   | CuDNN 8.0.5  | 7.1.3    |
| post11   |  11.0   | CuDNN 8.0.4  | 7.1.3    |

### How to make the compiler detect the CuDNN library

Download the corresponding CUDNN version from NVIDIA developer official website and decompressing it, add `-DCUDNN_ROOT` to cmake command, to specify the path of CUDNN.

## Compile and install OpenCV
**Note:** You need to do this only if you need to import the opencv library into your C + + code.

* First of all, you need to download the source code compiled package in the Linux environment from the OpenCV official website. Taking OpenCV3.4.7 as an example, the download command is as follows.

```
wget https://github.com/opencv/opencv/archive/3.4.7.tar.gz
tar -xf 3.4.7.tar.gz
```

Finally, you can see the folder of `opencv-3.4.7/` in the current directory.

* Compile OpenCV, the OpenCV source path (`root_path`) and installation path (`install_path`) should be set by yourself. Enter the OpenCV source code path and compile it in the following way.


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

Among them, `root_path` is the downloaded OpenCV source code path, and `install_path` is the installation path of OpenCV. After `make install` is completed, the OpenCV header file and library file will be generated in this folder for later source code compilation.



The final file structure under the OpenCV installation path is as follows.

```
opencv3/
|-- bin
|-- include
|-- lib
|-- lib64
|-- share
```
