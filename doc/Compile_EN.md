# How to compile PaddleServing

([简体中文](./Compile_CN.md)|English)

## Overview

Compiling Paddle Serving is divided into the following steps

- Compilation Environment Preparation: According to the needs of the model and operating environment, select the most suitable image
- Download the Serving Code Repo: Download the Serving code library, and perform initialization operations as needed
- Environment Variable Preparation: According to the needs of the running environment, determine the various environment variables of Python. For example, the GPU environment also needs to determine the environment variables such as Cuda, Cudnn, TensorRT and so on.
- Compilation: Compile `paddle-serving-server`, `paddle-serving-client`, `paddle-serving-app` related whl packages
- Install Related Whl Packages: install the three compiled whl packages, and set the SERVING_BIN environment variable

In addition, for some C++ secondary development scenarios, we also provide OPENCV binding solutions.

## Compilation Environment Requirements

|            module            |              version              |
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
|        python3-devel         |         3.6.0 and later |
|         sqlite-devel         |         3.7.17 and later          |
|           patchelf           |                0.9                |
|           libXext            |               1.3.3               |
|            libSM             |               1.2.2               |
|          libXrender          |              0.9.10               |

Docker compilation is recommended. We have prepared the Paddle Serving compilation environment for you and configured the above compilation dependencies. For details, please refer to [this document](DOCKER_IMAGES_CN.md).

We provide 4 environment development images, namely CPU, CUDA10.2 + CUDNN7, CUDA10.2 + CUDNN8, CUDA11.2 + CUDNN8. We provide a Serving development image to cover the above environment. At the same time, we also support Paddle development mirroring.

Serving development mirror is the mirror used to compile and debug prediction services provided by Serving suite in order to support various prediction environments. Paddle development mirror is the mirror used for compilation, development, and training models released by Paddle on the official website. In order to allow Paddle developers to use Serving directly in the same container. For developers who have already used Serving users in the previous version, Serving development image should not be unfamiliar. But for developers who are familiar with the Paddle training framework ecology, they should be more familiar with the existing Paddle development mirrors. In order to adapt to the different habits of all users, we have fully supported both sets of mirrors.

|  Environment           |   Serving Dev Image Tag               |    OS      | Paddle Dev Image Tag       |  OS            |
| :--------------------------: | :-------------------------------: | :-------------: | :-------------------: | :----------------: |
|  CPU                         | 0.9.0-devel                       |  Ubuntu 16.04   | 2.3.0                 | Ubuntu 18.04.       |
|  CUDA10.1 + Cudnn7             | 0.9.0-cuda10.1-cudnn7-devel       |  Ubuntu 16.04   | Nan                     | Nan                 |
|  CUDA10.2 + Cudnn8             | 0.9.0-cuda10.2-cudnn8-devel       |  Ubuntu 16.04   | Nan                    |  Nan                 |
|  CUDA11.2 + Cudnn8             | 0.9.0-cuda11.2-cudnn8-devel       |  Ubuntu 16.04   | 2.3.0-gpu-cuda11.2-cudnn8 | Ubuntu 18.04        | 

We first need to pull related images for the environment we need. Under the **Environment** column in the above table, except for the CPU, the rest (Cuda**+Cudnn**) belong to the GPU environment.

You can use Serving Dev Images.
```
docker pull registry.baidubce.com/paddlepaddle/serving:${Serving Dev Image Tag}

# For GPU Image
nvidia-docker run --rm -it registry.baidubce.com/paddlepaddle/serving:${Serving Dev Image Tag} bash

# For CPU Image
docker run --rm -it registry.baidubce.com/paddlepaddle/serving:${Serving Dev Image Tag} bash
```

You can also use Paddle Dev Images.
```
docker pull registry.baidubce.com/paddlepaddle/paddle:${Paddle Dev Image Tag}

# For GPU Image
nvidia-docker run --rm -it registry.baidubce.com/paddlepaddle/paddle:${Paddle Dev Image Tag} bash

# For CPU Image
docker run --rm -it registry.baidubce.com/paddlepaddle/paddle:${Paddle Dev Image Tag} bash
```

## Download the Serving Code Repo
**Note: If you are using Paddle to develop the image, you need to manually run `bash env_install.sh` after downloading the code base (as shown in the third line of the code box)**
```
git clone https://github.com/PaddlePaddle/Serving
cd Serving && git submodule update --init --recursive

# Paddle development image needs to run the following commands, Serving development image does not need to run
bash tools/paddle_env_install.sh
```

## Environment Variables Preparation

**Set PYTHON environment variable**

If you are using a Serving development image, please follow the steps below to determine the Python version that needs to be compiled and set the corresponding environment variables. A total of three environment variables need to be set, namely `PYTHON_INCLUDE_DIR`, `PYTHON_LIBRARIES`, `PYTHON_EXECUTABLE`. Below we take python 3.7 as an example to introduce how to set these three environment variables.

1) Set `PYTHON_INCLUDE_DIR`

Search the directory where Python.h is located
```
find / -name Python.h
```
Usually there will be something like `**/include/python3.7/Python.h`, we only need to take its folder directory, for example, find `/usr/include/python3.7/Python.h`, Then we only need `export PYTHON_INCLUDE_DIR=/usr/include/python3.7/`.
If not found. Explanation 1) The development version of Python is not installed and needs to be re-installed. 2) Insufficient permissions cannot view the relevant system directories.

2) Set `PYTHON_LIBRARIES`

Search for libpython3.7.so or libpython3.7m.so
```
find / -name libpython3.7.so
find / -name libpython3.7m.so
```
Usually there will be something similar to `**/lib/libpython3.7.so` or `**/lib/x86_64-linux-gnu/libpython3.7.so`, we only need to take its folder directory, For example, find `/usr/local/lib/libpython3.7.so`, then we only need `export PYTHON_LIBRARIES=/usr/local/lib`.
If it is not found, it means 1) Statically compiling Python, you need to reinstall the dynamically compiled Python 2) The county is not enough to view the relevant system catalogs.

3) Set `PYTHON_EXECUTABLE`

View the python3.7 path directly
```
which python3.7
```
If the result is `/usr/local/bin/python3.7`, then directly set `export PYTHON_EXECUTABLE=/usr/local/bin/python3.7`.

It is very important to set these three environment variables. After the settings are completed, we can perform the following operations (the following is the PYTHON environment of the development image of Paddle Cuda 11.2, if it is another image, please change the corresponding `PYTHON_INCLUDE_DIR`, `PYTHON_LIBRARIES` , `PYTHON_EXECUTABLE`).

```
# The following three environment variables are the environment of Paddle development mirror Cuda11.2, such as other mirrors may need to be modified
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

If you are a GPU user, you need to set additional `CUDA_PATH`, `CUDNN_LIBRARY`, `CUDA_CUDART_LIBRARY` and `TENSORRT_LIBRARY_PATH`.
```
export CUDA_PATH='/usr/local/cuda'
export CUDNN_LIBRARY='/usr/local/cuda/lib64/'
export CUDA_CUDART_LIBRARY="/usr/local/cuda/lib64/"
export TENSORRT_LIBRARY_PATH="/usr/"
```
The meaning of environment variables is shown in the table below.

| cmake environment variable | meaning | GPU environment considerations | whether Docker environment is needed |
|-----------------------|-------------------------------------|-------------------------------|--------------------|
| CUDA_TOOLKIT_ROOT_DIR | cuda installation path, usually /usr/local/cuda | Required for all environments | No (/usr/local/cuda) |
| CUDNN_LIBRARY | The directory where libcudnn.so.* is located, usually /usr/local/cuda/lib64/ | Required for all environments | No (/usr/local/cuda/lib64/) |
| CUDA_CUDART_LIBRARY | The directory where libcudart.so.* is located, usually /usr/local/cuda/lib64/ | Required for all environments | No (/usr/local/cuda/lib64/) |
| TENSORRT_ROOT | The upper level directory of the directory where libnvinfer.so.* is located, depends on the TensorRT installation directory | Required for all environments | No (/usr) |



## Compilation

We need to compile three targets in total, namely `paddle-serving-server`, `paddle-serving-client`, and `paddle-serving-app`, among which `paddle-serving-server` needs to distinguish between CPU or GPU version. If it is a CPU version, please run,

### Compile paddle-serving-server

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

If it is the GPU version, please run,
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

### Compile paddle-serving-client and paddle-serving-app

Next, we can continue to compile the client and app. The compilation commands for these two packages are common on all platforms, and do not distinguish between CPU and GPU versions.
```
# Compile paddle-serving-client
mkdir build_client
cd build_client
cmake -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR \
    -DPYTHON_LIBRARIES=$PYTHON_LIBRARIES \
    -DPYTHON_EXECUTABLE=$PYTHON_EXECUTABLE \
    -DCLIENT=ON ..
make -j10
cd ..

# Compile paddle-serving-app
mkdir build_app
cd build_app
cmake -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR \
    -DPYTHON_LIBRARIES=$PYTHON_LIBRARIES \
    -DPYTHON_EXECUTABLE=$PYTHON_EXECUTABLE \
    -DAPP=ON ..
make -j10
cd ..
```

## Install Related Whl Packages
```
pip3.7 install build_server/python/dist/*.whl
pip3.7 install build_client/python/dist/*.whl
pip3.7 install build_app/python/dist/*.whl
export SERVING_BIN=${PWD}/build_server/core/general-server/serving
```

## Precautions

Note the last line `export SERVING_BIN` in the previous section. When running the python server, the `SERVING_BIN` environment variable will be checked. If you want to use the binary file compiled by yourself, please set the environment variable to the path of the corresponding binary file. It is `export SERVING_BIN=${BUILD_DIR}/core/general-server/serving`.
Where BUILD_DIR is the absolute path of `build_server`.
You can cd build_server path and execute `export SERVING_BIN=${PWD}/core/general-server/serving`

## Enable WITH_OPENCV option to compile C++ Server

**Note:** You only need to do this when you need to do secondary development on the Paddle Serving C++ part and the newly added code depends on the OpenCV library.

To compile the Serving C++ Server part, when the WITH_OPENCV option is turned on, the installed OpenCV library is required. If it has not been installed, you can refer to the instructions at the back of this document to compile and install the OpenCV library.

Take the WITH_OPENCV option and compile the CPU version Paddle Inference Library as an example. On the basis of the above compilation command, add the `DOPENCV_DIR=${OPENCV_DIR}` and `DWITH_OPENCV=ON` options.
``` shell
OPENCV_DIR=your_opencv_dir #`your_opencv_dir` is the installation path of the opencv library.
mkdir build_server && cd build_server
cmake -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR/ \
    -DPYTHON_LIBRARIES=$PYTHON_LIBRARIES \
    -DPYTHON_EXECUTABLE=$PYTHON_EXECUTABLE \
    -DOPENCV_DIR=${OPENCV_DIR} \
    -DWITH_OPENCV=ON \
    -DSERVER=ON ..
make -j10
```

**Note:** After the compilation is successful, you need to set the `SERVING_BIN` path.





## Attached: CMake option description

| Compilation Options | Description | Default |
| :--------------: | :----------------------------------------: | :--: |
| WITH_AVX | Compile Paddle Serving with AVX intrinsics | OFF |
| WITH_MKL | Compile Paddle Serving with MKL support | OFF |
| WITH_GPU | Compile Paddle Serving with NVIDIA GPU | OFF |
| WITH_TRT | Compile Paddle Serving with TensorRT | OFF |
| WITH_OPENCV | Compile Paddle Serving with OPENCV | OFF |
| CUDNN_LIBRARY | Define CUDNN library and header path | |
| CUDA_TOOLKIT_ROOT_DIR | Define CUDA PATH | |
| TENSORRT_ROOT | Define TensorRT PATH | |
| CLIENT | Compile Paddle Serving Client | OFF |
| SERVER | Compile Paddle Serving Server | OFF |
| APP | Compile Paddle Serving App package | OFF |
| PACK | Compile for whl | OFF |

### WITH_GPU option

Paddle Serving supports prediction on the GPU through the PaddlePaddle prediction library. The WITH_GPU option is used to detect basic libraries such as CUDA/CUDNN on the system. If a suitable version is detected, the GPU version of the OP Kernel will be compiled when the PaddlePaddle is compiled.

To compile the Paddle Serving GPU version on bare metal, you need to install these basic libraries:

-CUDA
-CUDNN

To compile the TensorRT version, you need to install the TensorRT library.

The things to note here are:

1. Compile the basic library versions such as CUDA/CUDNN installed on the system where Serving is located, and need to be compatible with the actual GPU device. For example, Tesla V100 card requires at least CUDA 9.0. If the version of basic libraries such as CUDA used during compilation is too low, the Serving process cannot be started due to the incompatibility between the generated GPU code and the actual hardware device, or serious problems such as coredump may occur.
2. Install the CUDA driver compatible with the actual GPU device on the system running Paddle Serving, and install the basic library compatible with the CUDA/CUDNN version used during compilation. If the version of CUDA/CUDNN installed on the system running Paddle Serving is lower than the version used during compilation, it may cause strange cuda function call failures and other problems.

The following is the matching relationship between PaddleServing mirrored Cuda, Cudnn, and TensorRT for reference:

| Tag | CUDA | CUDNN | TensorRT |
| :----: | :-----: | :----------: | :----: |
| post101 | 10.1 | CUDNN 7.6.5 | 6.0.1 |
| post102 | 10.2 | CUDNN 8.0.5 | 7.1.3 |
| post11 | 11.0 | CUDNN 8.0.4 | 7.1.3 |

### Attachment: How to make the Paddle Serving compilation system detect the CUDNN library

After downloading the corresponding version of CUDNN from the official website of NVIDIA developer and decompressing it locally, add the `-DCUDNN_LIBRARY` parameter to the cmake compilation command and specify the path of the CUDNN library.

## Attachment: Compile and install OpenCV library
**Note:** You only need to do this when you need to include the OpenCV library in your C++ code.

* First, you need to download the package compiled from the source code in the Linux environment from the OpenCV official website. Take OpenCV 3.4.7 as an example. The download command is as follows.

```
wget https://github.com/opencv/opencv/archive/3.4.7.tar.gz
tar -xf 3.4.7.tar.gz
```

Finally, you can see the folder `opencv-3.4.7/` in the current directory.

* Compile OpenCV, set the OpenCV source path (`root_path`) and installation path (`install_path`). Enter the OpenCV source code path and compile in the following way.

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


Among them, `root_path` is the downloaded OpenCV source path, `install_path` is the installation path of OpenCV, after the completion of `make install`, OpenCV header files and library files will be generated in this folder, which are used to compile the code that references the OpenCV library .

The final file structure under the installation path is as follows.

```
opencv3/
|-- bin
|-- include
|-- lib
|-- lib64
|-- share
```
