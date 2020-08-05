# 如何编译PaddleServing

(简体中文|[English](./COMPILE.md))

## 编译环境设置

|             组件             |                           版本要求                           |
| :--------------------------: | :----------------------------------------------------------: |
|              OS              |                           CentOS 7                           |
|             gcc              |                       4.8.5 and later                        |
|           gcc-c++            |                       4.8.5 and later                        |
|             git              |                        3.82 and later                        |
|            cmake             |                       3.2.0 and later                        |
|            Python            |               2.7.2 and later / 3.6 and later                |
|              Go              |                       1.9.2 and later                        |
|             git              |                       2.17.1 and later                       |
|         glibc-static         |                             2.17                             |
|        openssl-devel         |                            1.0.2k                            |
|         bzip2-devel          |                       1.0.6 and later                        |
| python-devel / python3-devel |              2.7.5 and later / 3.6.8 and later               |
|         sqlite-devel         |                       3.7.17 and later                       |
|           patchelf           |                             0.9                              |
|           libXext            |                            1.3.3                             |
|            libSM             |                            1.2.2                             |
|          libXrender          |                            0.9.10                            |
|          python-whl          | numpy>=1.12, <=1.16.4<br/>google>=2.0.3<br/>protobuf>=3.12.2<br/>grpcio-tools>=1.28.1<br/>grpcio>=1.28.1<br/>func-timeout>=4.3.5<br/>pyyaml>=1.3.0<br/>sentencepiece==0.1.92<br/>flask>=1.1.2<br/>ujson>=2.0.3 |

推荐使用Docker编译，我们已经为您准备好了Paddle Serving编译环境，详见[该文档](DOCKER_IMAGES_CN.md)。

本文档将以Python2为例介绍如何编译Paddle Serving。如果您想用Python3进行编译，只需要调整cmake的Python相关选项即可：

- 将`DPYTHON_INCLUDE_DIR`设置为`$PYTHONROOT/include/python3.6m/`
- 将`DPYTHON_LIBRARIES`设置为`$PYTHONROOT/lib64/libpython3.6.so`
- 将`DPYTHON_EXECUTABLE`设置为`$PYTHONROOT/bin/python3.6`

## 获取代码

``` python
git clone https://github.com/PaddlePaddle/Serving
cd Serving && git submodule update --init --recursive
```




## PYTHONROOT设置

```shell
# 例如python的路径为/usr/bin/python，可以设置PYTHONROOT
export PYTHONROOT=/usr/
```

我们提供默认Centos7的Python路径为`/usr/bin/python`，如果您要使用我们的Centos6镜像，需要将其设置为`export PYTHONROOT=/usr/local/python2.7/`。



## 安装Python依赖

```shell
pip install -r python/requirements.txt
```

如果使用 Python3，请以 `pip3` 替换 `pip`。



## 编译Server部分

### 集成CPU版本Paddle Inference Library

``` shell
mkdir server-build-cpu && cd server-build-cpu
cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ -DPYTHON_LIBRARIES=$PYTHONROOT/lib/libpython2.7.so -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python -DSERVER=ON ..
make -j10
```

可以执行`make install`把目标产出放在`./output`目录下，cmake阶段需添加`-DCMAKE_INSTALL_PREFIX=./output`选项来指定存放路径。

### 集成GPU版本Paddle Inference Library

``` shell
mkdir server-build-gpu && cd server-build-gpu
cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ -DPYTHON_LIBRARIES=$PYTHONROOT/lib/libpython2.7.so -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python -DSERVER=ON -DWITH_GPU=ON ..
make -j10
```

执行`make install`可以把目标产出放在`./output`目录下。

**注意：** 编译成功后，需要设置`SERVING_BIN`路径，详见后面的[注意事项](https://github.com/PaddlePaddle/Serving/blob/develop/doc/COMPILE_CN.md#注意事项)。



## 编译Client部分

``` shell
mkdir client-build && cd client-build
cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ -DPYTHON_LIBRARIES=$PYTHONROOT/lib/libpython2.7.so -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python -DCLIENT=ON ..
make -j10
```

执行`make install`可以把目标产出放在`./output`目录下。



## 编译App部分

```bash
mkdir app-build && cd app-build
cmake -DPYTHON_INCLUDE_DIR=$PYTHONROOT/include/python2.7/ -DPYTHON_LIBRARIES=$PYTHONROOT/lib/libpython2.7.so -DPYTHON_EXECUTABLE=$PYTHONROOT/bin/python -DCMAKE_INSTALL_PREFIX=./output -DAPP=ON ..
make
```



## 安装wheel包

无论是Client端，Server端还是App部分，编译完成后，安装`python/dist/`下的whl包即可。



## 注意事项

运行python端Server时，会检查`SERVING_BIN`环境变量，如果想使用自己编译的二进制文件，请将设置该环境变量为对应二进制文件的路径，通常是`export SERVING_BIN=${BUILD_DIR}/core/general-server/serving`。



## CMake选项说明

|     编译选项     |                    说明                    | 默认 |
| :--------------: | :----------------------------------------: | :--: |
|     WITH_AVX     | Compile Paddle Serving with AVX intrinsics | OFF  |
|     WITH_MKL     |  Compile Paddle Serving with MKL support   | OFF  |
|     WITH_GPU     |   Compile Paddle Serving with NVIDIA GPU   | OFF  |
|    CUDNN_ROOT    |    Define CuDNN library and header path    |      |
|      CLIENT      |       Compile Paddle Serving Client        | OFF  |
|      SERVER      |       Compile Paddle Serving Server        | OFF  |
|       APP        |     Compile Paddle Serving App package     | OFF  |
| WITH_ELASTIC_CTR |        Compile ELASITC-CTR solution        | OFF  |
|       PACK       |              Compile for whl               | OFF  |

### WITH_GPU选项

Paddle Serving通过PaddlePaddle预测库支持在GPU上做预测。WITH_GPU选项用于检测系统上CUDA/CUDNN等基础库，如检测到合适版本，在编译PaddlePaddle时就会编译出GPU版本的OP Kernel。

在裸机上编译Paddle Serving GPU版本，需要安装这些基础库：

- CUDA
- CuDNN
- NCCL2

这里要注意的是：

1. 编译Serving所在的系统上所安装的CUDA/CUDNN等基础库版本，需要兼容实际的GPU设备。例如，Tesla V100卡至少要CUDA 9.0。如果编译时所用CUDA等基础库版本过低，由于生成的GPU代码和实际硬件设备不兼容，会导致Serving进程无法启动，或出现coredump等严重问题。
2. 运行Paddle Serving的系统上安装与实际GPU设备兼容的CUDA driver，并安装与编译期所用的CUDA/CuDNN等版本兼容的基础库。如运行Paddle Serving的系统上安装的CUDA/CuDNN的版本低于编译时所用版本，可能会导致奇怪的cuda函数调用失败等问题。

以下是PaddlePaddle发布版本所使用的基础库版本匹配关系，供参考：

|        |  CUDA   |          CuDNN           | NCCL2  |
| :----: | :-----: | :----------------------: | :----: |
| CUDA 8 | 8.0.61  | CuDNN 7.1.2 for CUDA 8.0 | 2.1.4  |
| CUDA 9 | 9.0.176 | CuDNN 7.3.1 for CUDA 9.0 | 2.2.12 |

### 如何让Paddle Serving编译系统探测到CuDNN库

从NVIDIA developer官网下载对应版本CuDNN并在本地解压后，在cmake编译命令中增加`-DCUDNN_ROOT`参数，指定CuDNN库所在路径。

### 如何让Paddle Serving编译系统探测到nccl库

从NVIDIA developer官网下载对应版本nccl2库并解压后，增加如下环境变量 (以nccl2.1.4为例)：

```shell
export C_INCLUDE_PATH=/path/to/nccl2/cuda8/nccl_2.1.4-1+cuda8.0_x86_64/include:$C_INCLUDE_PATH
export CPLUS_INCLUDE_PATH=/path/to/nccl2/cuda8/nccl_2.1.4-1+cuda8.0_x86_64/include:$CPLUS_INCLUDE_PATH
export LD_LIBRARY_PATH=/path/to/nccl2/cuda8/nccl_2.1.4-1+cuda8.0_x86_64/lib/:$LD_LIBRARY_PATH
```
