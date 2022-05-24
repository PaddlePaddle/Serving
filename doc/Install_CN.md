# 使用Docker安装Paddle Serving

(简体中文|[English](./Install_EN.md))

- [1.使用开发镜像](#1)
    - [Serving 开发镜像](#1.1)
        - [CPU 镜像](#1.1.1)
        - [GPU 镜像](#1.1.2)
        - [ARM & XPU 镜像](#1.1.3)
    - [Paddle 开发镜像](#1.2)
        - [CPU 镜像](#1.2.1)
        - [GPU 镜像](#1.2.2)
- [2.安装 Wheel 包](#2)
    - [在线安装](#2.1)
    - [离线安装](#2.2)
    - [ARM & XPU 包安装](#2.3)
- [3.环境检查](#3)


**强烈建议**您在**Docker内构建**Paddle Serving，更多镜像请查看[Docker镜像列表](Docker_Images_CN.md)。

**提示-1**：本项目仅支持<mark>**Python3.6/3.7/3.8/3.9**</mark>，接下来所有的与Python/Pip相关的操作都需要选择正确的Python版本。

**提示-2**：以下示例中GPU环境均为cuda11.2-cudnn8，如果您使用Python Pipeline来部署，并需要Nvidia TensorRT来优化预测性能，请参考以下说明来选择其他版本。

<a name="1"></a>

## 1.使用开发镜像

- Serving 镜像: registry.baidubce.com/paddlepaddle/serving:{Tag}
- Paddle 镜像: registry.baidubce.com/paddlepaddle/paddle:{Tag}

<mark>**同时支持使用 Serving 镜像和 Paddle 镜像，`1.1` 和 `1.2` 章节中的操作2选1即可。**</mark> 在Paddle docker镜像上部署 Servin g服务需要安装额外依赖库，因此，我们直接使用 Serving 开发镜像。

|  环境                         |   Serving镜像 Tag               |    操作系统      | Paddle镜像 Tag       |  操作系统            |
| :--------------------------: | :-------------------------------: | :-------------: | :-------------------: | :----------------: |
|  CPU                         | 0.9.0-devel                       |  Ubuntu 16    | 2.3.0                | Ubuntu 18       |
|  CUDA10.1 + cuDNN 7           | 0.9.0-cuda10.1-cudnn7-devel       |  Ubuntu 16   | 无                    | 无                 |
|  CUDA10.2 + cuDNN 7           | 0.9.0-cuda10.2-cudnn7-devel       |  Ubuntu 16   | 2.3.0-gpu-cuda10.2-cudnn7 | Ubuntu 18
|  CUDA10.2 + cuDNN 8           | 0.9.0-cuda10.2-cudnn8-devel       |  Ubuntu 16   | 无                   | Ubuntu 18 |
|  CUDA11.2 + cuDNN 8           | 0.9.0-cuda11.2-cudnn8-devel       |  Ubuntu 16   | 2.3.0-gpu-cuda11.2-cudnn8 | Ubuntu 18   | 
|  ARM + XPU                    | xpu-arm                           |  CentOS 8.3  | 无                         | 无           |

对于**Windows 10 用户**，请参考文档[Windows平台使用Paddle Serving指导](Windows_Tutorial_CN.md)。

<a name="1.1"></a>

### 1.1 Serving开发镜像（CPU/GPU 2选1）

<a name="1.1.1"></a>

**CPU：**
```
# 启动 CPU Docker
docker pull registry.baidubce.com/paddlepaddle/serving:0.9.0-devel
docker run -p 9292:9292 --name test_cpu -dit registry.baidubce.com/paddlepaddle/serving:0.9.0-devel bash
docker exec -it test_cpu bash
git clone https://github.com/PaddlePaddle/Serving
```

<a name="1.1.2"></a>

**GPU：**
```
# 启动 GPU Docker
docker pull registry.baidubce.com/paddlepaddle/serving:0.9.0-cuda11.2-cudnn8-devel
nvidia-docker run -p 9292:9292 --name test_gpu -dit registry.baidubce.com/paddlepaddle/serving:0.9.0-cuda11.2-cudnn8-devel bash
nvidia-docker exec -it test_gpu bash
git clone https://github.com/PaddlePaddle/Serving
```

<a name="1.1.3"></a>

**ARM & XPU: **
```
docker pull registry.baidubce.com/paddlepaddle/serving:xpu-arm
docker run -p 9292:9292 --name test_arm_xpu -dit registry.baidubce.com/paddlepaddle/serving:xpu-arm bash
docker exec -it test_arm_xpu bash
git clone https://github.com/PaddlePaddle/Serving
```

<a name="1.2"></a>

### 1.2 Paddle开发镜像（CPU/GPU 2选1）

<a name="1.2.1"></a>

**CPU：**
```
### 启动 CPU Docker
nvidia-docker pull registry.baidubce.com/paddlepaddle/paddle:2.3.0
docker run -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/paddle:2.3.0 bash
docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving

### Paddle开发镜像需要执行以下脚本增加Serving所需依赖项
bash Serving/tools/paddle_env_install.sh
```

<a name="1.2.2"></a>

**GPU：**
```
### 启动 GPU Docker

nvidia-docker pull registry.baidubce.com/paddlepaddle/paddle:2.3.0-gpu-cuda11.2-cudnn8
nvidia-docker run -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/paddle:2.3.0-gpu-cuda11.2-cudnn8 bash
nvidia-docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving

### Paddle开发镜像需要执行以下脚本增加Serving所需依赖项
bash Serving/tools/paddle_env_install.sh
```

<a name="2"></a>

## 2.安装 Wheel 包

安装所需的pip依赖
```
cd Serving
pip3 install -r python/requirements.txt
```

安装服务whl包，共有3种client、app、server，Server分为CPU和GPU，GPU包根据您的环境选择一种安装
- post112 = CUDA11.2 + cuDNN8 + TensorRT8（推荐）
- post101 = CUDA10.1 + cuDNN7 + TensorRT6
- post102 = CUDA10.2 + cuDNN7 + TensorRT6 (与Paddle 镜像一致)
- post1028 = CUDA10.2 + cuDNN8 + TensorRT7

<a name="2.1"></a>

### 2.1 在线安装
在线安装采用 `pypi` 下载并安装的方式。

```shell
pip3 install paddle-serving-client==0.9.0 -i https://pypi.tuna.tsinghua.edu.cn/simple
pip3 install paddle-serving-app==0.9.0 -i https://pypi.tuna.tsinghua.edu.cn/simple

# CPU Server
pip3 install paddle-serving-server==0.9.0 -i https://pypi.tuna.tsinghua.edu.cn/simple

# GPU Server，需要确认环境再选择执行哪一条，推荐使用CUDA 11.2的包
pip3 install paddle-serving-server-gpu==0.9.0.post112 -i https://pypi.tuna.tsinghua.edu.cn/simple
```

默认开启国内清华镜像源来加速下载，如果您使用HTTP代理可以关闭(`-i https://pypi.tuna.tsinghua.edu.cn/simple`)

如果需要使用develop分支编译的安装包，请从[下载Wheel包](./Latest_Packages_CN.md)中获取下载地址进行下载，使用`pip install`命令进行安装。如果您想自行编译，请参照[Paddle Serving编译文档](./Compile_CN.md)。

`paddle-serving-server` 和 `paddle-serving-server-gpu` 安装包支持Centos 6/7, Ubuntu 16/18和Windows 10。

`paddle-serving-client` 和 `paddle-serving-app` 安装包支持 Linux 和 Windows，其中 `paddle-serving-client` 仅支持 python3.6/3.7/3.8/3.9。


**当您使用`paddle_serving_client.convert`命令或者`Python Pipeline框架`时才需要安装。**
```
# CPU环境请执行
pip3 install paddlepaddle==2.3.0 -i https://pypi.tuna.tsinghua.edu.cn/simple

# GPU CUDA 11.2环境请执行
pip3 install paddlepaddle-gpu==2.3.0.post112 -i https://pypi.tuna.tsinghua.edu.cn/simple
```
**注意**： 其他版本请参考[Paddle-Inference官方文档-下载安装Linux预测库](https://paddleinference.paddlepaddle.org.cn/master/user_guides/download_lib.html#python) 选择相应的GPU环境的 URL 链接并进行安装。

```
# CUDA11.2 + CUDNN8 + TensorRT8 + Python(3.6-3.9)
pip3 install https://paddle-inference-lib.bj.bcebos.com/2.3.0/python/Linux/GPU/x86-64_gcc8.2_avx_mkl_cuda11.2_cudnn8.2.1_trt8.0.3.4/paddlepaddle_gpu-2.3.0.post112-cp36-cp36m-linux_x86_64.whl
pip3 install https://paddle-inference-lib.bj.bcebos.com/2.3.0/python/Linux/GPU/x86-64_gcc8.2_avx_mkl_cuda11.2_cudnn8.2.1_trt8.0.3.4/paddlepaddle_gpu-2.3.0.post112-cp37-cp37m-linux_x86_64.whl
pip3 install https://paddle-inference-lib.bj.bcebos.com/2.3.0/python/Linux/GPU/x86-64_gcc8.2_avx_mkl_cuda11.2_cudnn8.2.1_trt8.0.3.4/paddlepaddle_gpu-2.3.0.post112-cp38-cp38-linux_x86_64.whl
pip3 install https://paddle-inference-lib.bj.bcebos.com/2.3.0/python/Linux/GPU/x86-64_gcc8.2_avx_mkl_cuda11.2_cudnn8.2.1_trt8.0.3.4/paddlepaddle_gpu-2.3.0.post112-cp39-cp39-linux_x86_64.whl

# CUDA10.1 + CUDNN7 + TensorRT6 + Python(3.6-3.9)
pip3 install https://paddle-inference-lib.bj.bcebos.com/2.3.0/python/Linux/GPU/x86-64_gcc8.2_avx_mkl_cuda10.1_cudnn7.6.5_trt6.0.1.5/paddlepaddle_gpu-2.3.0.post101-cp36-cp36m-linux_x86_64.whl
pip3 install https://paddle-inference-lib.bj.bcebos.com/2.3.0/python/Linux/GPU/x86-64_gcc8.2_avx_mkl_cuda10.1_cudnn7.6.5_trt6.0.1.5/paddlepaddle_gpu-2.3.0.post101-cp37-cp37m-linux_x86_64.whl
pip3 install https://paddle-inference-lib.bj.bcebos.com/2.3.0/python/Linux/GPU/x86-64_gcc8.2_avx_mkl_cuda10.1_cudnn7.6.5_trt6.0.1.5/paddlepaddle_gpu-2.3.0.post101-cp38-cp38-linux_x86_64.whl
pip3 install https://paddle-inference-lib.bj.bcebos.com/2.3.0/python/Linux/GPU/x86-64_gcc8.2_avx_mkl_cuda10.1_cudnn7.6.5_trt6.0.1.5/paddlepaddle_gpu-2.3.0.post101-cp39-cp39-linux_x86_64.whl

# CUDA10.2 + CUDNN8 + TensorRT7 + Python(3.6-3.9)
pip3 install https://paddle-inference-lib.bj.bcebos.com/2.3.0/python/Linux/GPU/x86-64_gcc8.2_avx_mkl_cuda10.2_cudnn8.1.1_trt7.2.3.4/paddlepaddle_gpu-2.3.0-cp36-cp36m-linux_x86_64.whl
pip3 install https://paddle-inference-lib.bj.bcebos.com/2.3.0/python/Linux/GPU/x86-64_gcc8.2_avx_mkl_cuda10.2_cudnn8.1.1_trt7.2.3.4/paddlepaddle_gpu-2.3.0-cp37-cp37m-linux_x86_64.whl
pip3 install https://paddle-inference-lib.bj.bcebos.com/2.3.0/python/Linux/GPU/x86-64_gcc8.2_avx_mkl_cuda10.2_cudnn8.1.1_trt7.2.3.4/paddlepaddle_gpu-2.3.0-cp38-cp38-linux_x86_64.whl
pip3 install https://paddle-inference-lib.bj.bcebos.com/2.3.0/python/Linux/GPU/x86-64_gcc8.2_avx_mkl_cuda10.2_cudnn8.1.1_trt7.2.3.4/paddlepaddle_gpu-2.3.0-cp39-cp39-linux_x86_64.whl

```

<a name="2.2"></a>

### 2.2 离线安装
离线安装是指所有的 Paddle 和 Serving 包和依赖库，传入到无网或弱网环境下安装。

**1.安装离线 Wheel 包**

Serving 和 Paddle Wheel包的离线依赖包下载有4个链接。

```
wget https://paddle-serving.bj.bcebos.com/offline_wheels/0.9.0/py36_offline_whl_packages.tar
wget https://paddle-serving.bj.bcebos.com/offline_wheels/0.9.0/py37_offline_whl_packages.tar
wget https://paddle-serving.bj.bcebos.com/offline_wheels/0.9.0/py38_offline_whl_packages.tar
wget https://paddle-serving.bj.bcebos.com/offline_wheels/0.9.0/py39_offline_whl_packages.tar
```

通过运行 `install.py` 脚本可本地安装 Serving 和 Paddle Wheel 包。`install.py` 脚本的参数列表如下：
```
python3 install.py
  --python_version : Python version for installing wheels, one of [py36, py37, py38, py39], py37 default.
  --device : Type of devices, one of [cpu, gpu], cpu default.
  --cuda_version : CUDA version for GPU, one of [101, 102, 112, empty], empty default.
  --serving_version : Verson of Serving, one of [0.9.0, no_install], 0.9.0 default.
  --paddle_version Verson of Paddle, one of [2.3.0, no_install], 2.3.0 default.
```

**2.在环境变量中指定 `SERVING_BIN` 路径**

完成第1步安装后，若仅使用 python pipeline 模式可忽略此步骤。

如使用 C++ Serving 使用命令行方式启动服务，示例如下。则需要在命令行窗口或服务启动程序中导出环境变量 `SERVING_BIN`，使用本地的 serving 二进制程序运行服务。

C++ Serving 命令行启动服务示例：
```
python3 -m paddle_serving_server.serve --model serving_model --thread 10 --port 9292 --gpu_ids 0,1,2
```

由于所有版本的二进制程序包有 20 GB，非常大。因此提供多个版本的下载链接，通过手动 `wget` 下载指定版本到 `serving_bin` 目录下，解压后导出到环境变量中。

- cpu-avx-mkl: https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-cpu-avx-mkl-0.9.0.tar.gz
- cpu-avx-openblas: https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-cpu-avx-openblas-0.9.0.tar.gz
- cpu-noavx-openblas: https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-cpu-noavx-openblas-0.9.0.tar.gz
- cuda10.1-cudnn7-TensorRT6: https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-gpu-101-0.9.0.tar.gz
- cuda10.2-cudnn7-TensorRT6: https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-gpu-102-0.9.0.tar.gz
- cuda10.2-cudnn8-TensorRT7: https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-gpu-1028-0.9.0.tar.gz
- cuda11.2-cudnn8-TensorRT8: https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-gpu-112-0.9.0.tar.gz

以 GPU CUDA 10.2 为例，在命令行或启动程序中设置环境变量如下：
```
export SERVING_BIN = $PWD/serving_bin/serving-gpu-102-0.9.0/serving
```

**3.运行 `install.py` 安装 Wheel 包**

1.同时安装 Serving 和 Paddle 的 py38 版本 GPU wheel 包：
```
python3 install.py --cuda_version="102" --python_version="py38" --device="GPU" --serving_version="0.9.0" --paddle_version="2.3.0"
```

2.仅安装 Serving 的 py39 版本 CPU wheel 包，设置 `--paddle_version="no_install"` 表示不安装 Paddle 预测库，设置 `--device="cpu"` 表示 cpu 版本
```
python3 install.py --cuda_version="" --python_version="py39" --device="cpu" --serving_version="0.9.0" --paddle_version="no_install"
```

3.仅安装 Paddle 的 py36 版本`cuda=11.2` 的 GPU wheel 包，
```
python3 install.py --cuda_version="112" --python_version="py36" --device="GPU" --serving_version="no_install" --paddle_version="2.3.0"
```

<a name="2.3"></a>

### 2.3 ARM & XPU 安装 wheel 包

由于使用 ARM 和 XPU 的用户较少，安装此环境的 Wheel 单独提供如下，其中 `paddle_serving_client` 仅提供 `py36` 的版本，如需其他版本请与我们联系。

```
pip3.6 install https://paddle-serving.bj.bcebos.com/test-dev/whl/arm/paddle_serving_app-0.9.0-py3-none-any.whl
pip3.6 install https://paddle-serving.bj.bcebos.com/test-dev/whl/arm/paddle_serving_client-0.9.0-cp36-none-any.whl
pip3.6 install https://paddle-serving.bj.bcebos.com/test-dev/whl/arm/paddle_serving_server_xpu-0.9.0.post2-py3-none-any.whl
```

二进制包地址:
```
wget https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-xpu-aarch64-0.9.0.tar.gz
```


<a name="3"></a>

## 3.环境检查
当以上步骤均完成后可使用命令行运行环境检查功能，自动运行Paddle Serving相关示例，进行环境相关配置校验。
```
python3 -m paddle_serving_server.serve check
```
详情请参考[环境检查文档](./Check_Env_CN.md)