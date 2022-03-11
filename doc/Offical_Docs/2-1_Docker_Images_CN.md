# Docker 镜像安装

本文档介绍基于标准镜像环境的安装流程。

## 获取镜像

**一. CPU/GPU 镜像：**

您可以通过两种方式获取镜像。

1. 通过 TAG 直接从 dockerhub 或 `registry.baidubce.com` 拉取镜像，具体 TAG 请参见下文的镜像说明章节的表格**

   ```shell
   docker pull registry.baidubce.com/paddlepaddle/serving:<TAG> 
   ```

2. 基于 Dockerfile 构建镜像**

   建立新目录，复制对应 Dockerfile 内容到该目录下 Dockerfile 文件。执行

   ```shell
   docker build -f tools/${DOCKERFILE} -t <image-name>:<images-tag> .
   ```

若需要基于源代码二次开发编译，请使用后缀为 -devel 的版本。


|                         镜像选择                         |   操作系统    |             TAG              |                          Dockerfile                          |
| :----------------------------------------------------------: | :-----: | :--------------------------: | :----------------------------------------------------------: |
|   CPU development     | Ubuntu16 |         0.8.0-devel         |        [Dockerfile.devel](../tools/Dockerfile.devel)         |
|   GPU (cuda10.1-cudnn7-tensorRT6-gcc54) development   | Ubuntu16 | 0.8.0-cuda10.1-cudnn7-gcc54-devel (not ready) | [Dockerfile.cuda10.1-cudnn7-gcc54.devel](../tools/Dockerfile.cuda10.1-cudnn7-gcc54.devel) |
|  GPU (cuda10.1-cudnn7-tensorRT6) development  | Ubuntu16 | 0.8.0-cuda10.1-cudnn7-devel | [Dockerfile.cuda10.1-cudnn7.devel](../tools/Dockerfile.cuda10.1-cudnn7.devel) |
|  GPU (cuda10.2-cudnn7-tensorRT6) development  | Ubuntu16 | 0.8.0-cuda10.2-cudnn7-devel | [Dockerfile.cuda10.2-cudnn7.devel](../tools/Dockerfile.cuda10.2-cudnn7.devel) |
| GPU (cuda10.2-cudnn8-tensorRT7) development  | Ubuntu16 | 0.8.0-cuda10.2-cudnn8-devel | [Dockerfile.cuda10.2-cudnn8.devel](../tools/Dockerfile.cuda10.2-cudnn8.devel) |
|  GPU (cuda11.2-cudnn8-tensorRT8) development  | Ubuntu16 | 0.8.0-cuda11.2-cudnn8-devel | [Dockerfile.cuda11.2-cudnn8.devel](../tools/Dockerfile.cuda11.2-cudnn8.devel) |
|   CPU Runtime |Ubuntu 16|0.8.0-runtime||
|   GPU (cuda10.1-cudnn7-tensorRT6) Runtime |Ubuntu 16|0.8.0-cuda10.1-cudnn7-runtime||
|   GPU (cuda10.2-cudnn8-tensorRT7) Runtime |Ubuntu 16|0.8.0-cuda10.2-cudnn8-runtime||
|   GPU (cuda11.2-cudnn8-tensorRT8) Runtime |Ubuntu 16|0.8.0-cuda11.2-cudnn8-runtime||

**二. Java 镜像：**
```
docker pull registry.baidubce.com/paddlepaddle/serving:0.8.0-cuda10.2-java
```

**三. XPU 镜像：**
```
docker pull registry.baidubce.com/paddlepaddle/serving:xpu-arm # for arm xpu user
docker pull registry.baidubce.com/paddlepaddle/serving:xpu-x86 # for x86 xpu user
```

**四. ROCM 镜像：**
```
docker pull paddlepaddle/paddle:latest-dev-rocm4.0-miopen2.11 # for x86 rocm user
```

**五. NPU 镜像：**
```
docker pull paddlepaddle/paddle:latest-dev-cann5.0.2.alpha005-gcc82-aarch64 # for arm ascend910 user
docker pull registry.baidubce.com/paddlepaddle/serving:ascend-aarch64-cann3.3.0-paddlelite-devel # for arm ascend310 user
```

## 使用 pip 安装

本节将介绍使用 pip 的安装方式。
以下示例中 GPU 环境均为 cuda10.2-cudnn7

**一. 启动开发镜像**

    CPU
    ```
    # 启动 CPU Docker
    docker pull paddlepaddle/serving:0.8.0-devel
    docker run -p 9292:9292 --name test -dit paddlepaddle/serving:0.8.0-devel bash
    docker exec -it test bash
    git clone https://github.com/PaddlePaddle/Serving
    ```
    GPU
    ```
    # 启动 GPU Docker
    docker pull paddlepaddle/serving:0.8.0-cuda10.2-cudnn7-devel
    nvidia-docker run -p 9292:9292 --name test -dit paddlepaddle/serving:0.8.0-cuda10.2-cudnn7-devel bash
    nvidia-docker exec -it test bash
    git clone https://github.com/PaddlePaddle/Serving
    ```

**二. 安装所需的 pip 依赖**

    ```
    cd Serving
    pip3 install -r python/requirements.txt
    ```

**三. 安装服务 whl 包**

   共有3种 client、app、server，Server 分为 CPU 和 GPU，GPU 包根据您的环境选择一种安装

    - post102 = CUDA10.2 + Cudnn7 + TensorRT6（推荐）
    - post101 = CUDA10.1 + TensorRT6
    - post112 = CUDA11.2 + TensorRT8

    ```shell
    pip3 install paddle-serving-client==0.8.3 -i https://pypi.tuna.tsinghua.edu.cn/simple
    pip3 install paddle-serving-app==0.8.3 -i https://pypi.tuna.tsinghua.edu.cn/simple
    
    # CPU Server
    pip3 install paddle-serving-server==0.8.3 -i https://pypi.tuna.tsinghua.edu.cn/simple
    
    # GPU Server，需要确认环境再选择执行哪一条，推荐使用CUDA 10.2的包
    pip3 install paddle-serving-server-gpu==0.8.3.post102 -i https://pypi.tuna.tsinghua.edu.cn/simple 
    pip3 install paddle-serving-server-gpu==0.8.3.post101 -i https://pypi.tuna.tsinghua.edu.cn/simple
    pip3 install paddle-serving-server-gpu==0.8.3.post112 -i https://pypi.tuna.tsinghua.edu.cn/simple
    ```

    默认开启国内清华镜像源来加速下载，如果您使用 HTTP 代理可以关闭(`-i https://pypi.tuna.tsinghua.edu.cn/simple`)

**四. 安装 Paddle 相关 Python 库**
    **当您使用`paddle_serving_client.convert`命令或者`Python Pipeline 框架`时才需要安装。**
    ```
    # CPU 环境请执行
    pip3 install paddlepaddle==2.2.2

    # GPU CUDA 10.2环境请执行
    pip3 install paddlepaddle-gpu==2.2.2
    ```
    **注意**： 如果您的 Cuda 版本不是10.2，或者您需要在 GPU 环境上使用 TensorRT，请勿直接执行上述命令，需要参考[Paddle-Inference官方文档-下载安装Linux预测库](https://paddleinference.paddlepaddle.org.cn/master/user_guides/download_lib.html#python)选择相应的 GPU 环境的 url 链接并进行安装。

**五. 安装完成后的环境检查**
    当以上步骤均完成后可使用命令行运行环境检查功能，自动运行 Paddle Serving 相关示例，进行环境相关配置校验。

    ```
    python3 -m paddle_serving_server.serve check
    ```

    详情请参考[环境检查文档](./Check_Env_CN.md)
