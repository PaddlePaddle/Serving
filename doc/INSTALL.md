# Install

## 系统需求

OS: Linux

CMake: (验证过的版本：3.2)

C++编译器 (验证过的版本：GCC 4.8.2/5.4.0)

python (验证过的版本：2.7)

Go编译器 (验证过的版本：1.9.2/1.12.0)

openssl & openssl-devel

## 编译

推荐使用docker编译Paddle Serving, [镜像dockerfile](./DOCKER.md)

```shell
$ git clone https://github.com/PaddlePaddle/serving.git
$ cd serving
$ mkdir build
$ cd build
$ cmake ..
$ make -j4
$ make install
```

`make install`将把目标产出放在/path/to/paddle-serving/build/output/目录下，目录结构：

```
.
|-- bin                             # Paddle Serving protobuf编译插件pdcodegen所在目录
|-- demo                            # demo总目录
|   |-- client
|   |   |-- dense_format            # dense_format客户端
|   |   |   |-- bin                 # bin/dense_format是dense_format客户端bin
|   |   |   `-- conf
|   |   |-- echo                    # echo服务客户端
|   |   |   |-- bin                 # bin/echo是echo客户端bin
|   |   |   \-- conf
|   |   |-- image_classification    # image_classification服务客户端
|   |   |   |-- bin                 # bin/ximage是image_classification客户端bin
|   |   |   |-- conf
|   |   |   |-- data
|   |   |      `-- images
|   |   |-- int64tensor_format      # int64tensor_format服务客户端
|   |   |   |-- bin                 # bin/int64tensor_format是客户端bin
|   |   |   `-- conf
|   |   `-- sparse_format           # sparse_format客户端
|   |       |-- bin                 # bin/sparse_format是客户端bin
|   |       `-- conf
|   `-- serving                     # serving端，同时提供echo/dense_format/sparse_format/int64tensor_format/image_class等5种服务
|       |-- bin                     # bin/serving是serving端可执行bin
|       |-- conf                    # 配置文件目录
|       |-- data
|       |   `-- model
|       |       `-- paddle
|       |           `-- fluid
|       |               `-- SE_ResNeXt50_32x4d  # image_classification模型
`-- lib                             # Paddle Serving产出的静态库文件: libpdseving.a, libsdk-cpp.a, libconfigure.a, libfluid_cpu_engine.a
```

如要编写新的预测服务，请参考[从零开始写一个预测服务](CREATING.md)

# CMake编译选项说明

| 编译选项 | 说明 |
|----------|------|
| WITH_AVX | For configuring PaddlePaddle. Compile PaddlePaddle with AVX intrinsics |
| WITH_MKL | For configuring PaddlePaddle. Compile PaddlePaddle with MKLML library |
| WITH_GPU | For configuring PaddlePaddle. Compile PaddlePaddle with NVIDIA GPU |
| CUDNN_ROOT| For configuring PaddlePaddle. Define CuDNN library and header path |
| CLINET_ONLY | Compile client libraries and demos only |

## WITH_GPU选项

Paddle Serving通过PaddlePaddle预测库支持在GPU上做预测。WITH_GPU选项用于检测系统上CUDA/CUDNN等基础库，如检测到合适版本，在编译PaddlePaddle时就会编译出GPU版本的OP Kernel。

在裸机上编译Paddle Serving GPU版本，需要安装这些基础库：

- CUDA
- CuDNN
- NCCL2

这里要注意的是：
1) 编译Serving所在的系统上所安装的CUDA/CUDNN等基础库版本，需要兼容实际的GPU设备。例如，Tesla V100卡至少要CUDA 9.0。如果编译时所用CUDA等基础库版本过低，由于生成的GPU代码和实际硬件设备不兼容，会导致Serving进程无法启动，或出现coredump等严重问题。
2) 运行Paddle Serving的系统上安装与实际GPU设备兼容的CUDA driver，并安装与编译期所用的CUDA/CuDNN等版本兼容的基础库。如运行Paddle Serving的系统上安装的CUDA/CuDNN的版本低于编译时所用版本，可能会导致奇怪的cuda函数调用失败等问题。

以下是PaddlePaddle发布版本所使用的基础库版本匹配关系，供参考：

| | CUDA  | CuDNN | NCCL2 |
|-|-------|--------------------------|-------|
| CUDA 8 | 8.0.61 | CuDNN 7.1.2 for CUDA 8.0 | 2.1.4 |
| CUDA 9 | 9.0.176 | CuDNN 7.3.1 for CUDA 9.0| 2.2.12 |

### 如何让Paddle Serving编译系统探测到CuDNN库

从NVIDIA developer官网下载对应版本CuDNN并在本地解压后，在cmake编译命令中增加-DCUDNN_ROOT参数，指定CuDNN库所在路径：

```
$ pwd
/path/to/paddle-serving

$ mkdir build && cd build
$ cmake -DWITH_GPU=ON -DCUDNN_ROOT=/path/to/cudnn/cudnn_v7/cuda ..
```

### 如何让Paddle Serving编译系统探测到nccl库

从NVIDIA developer官网下载对应版本nccl2库并解压后，增加如下环境变量 (以nccl2.1.4为例)：

```
$ export C_INCLUDE_PATH=/path/to/nccl2/cuda8/nccl_2.1.4-1+cuda8.0_x86_64/include:$C_INCLUDE_PATH
$ export CPLUS_INCLUDE_PATH=/path/to/nccl2/cuda8/nccl_2.1.4-1+cuda8.0_x86_64/include:$CPLUS_INCLUDE_PATH
$ export LD_LIBRARY_PATH=/path/to/nccl2/cuda8/nccl_2.1.4-1+cuda8.0_x86_64/lib/:$LD_LIBRARY_PATH
```
