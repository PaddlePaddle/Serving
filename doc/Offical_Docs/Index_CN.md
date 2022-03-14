# 安装指南

##  安装说明

本说明将指导您在64位操作系统编译和安装 PaddleServing。
**强烈建议**您在 **Docker 内构建** Paddle Serving，更多镜像请查看 [Docker镜像列表](Docker_Images_CN.md)。

**一. Python 和 pip 版本：**

* Python 的版本支持 3.6/3.7/3.8/3.9
* Python 具有 pip, 且 pip 的版本要求 20.2.2+
* Python 和 pip 要求是 64 位版本

**二. PaddleServing 对 GPU 支持情况：**

* 目前 **PaddleServing** 支持 **NVIDIA** 显卡的 **CUDA** 驱动和 **AMD** 显卡的 **ROCm** 架构
* 目前支持CUDA 10.1/10.2/11.2


**第一种安装方式：使用 pip 安装**

您可以选择“使用 pip 安装”、“从源码编译安装” 两种方式中的任意一种方式进行安装。

本节将介绍使用 pip 的安装方式。
以下示例中 GPU 环境均为 cuda10.2-cudnn7

1. 启动开发镜像

    **CPU：**
    ```
    # 启动 CPU Docker
    docker pull paddlepaddle/serving:0.8.0-devel
    docker run -p 9292:9292 --name test -dit paddlepaddle/serving:0.8.0-devel bash
    docker exec -it test bash
    git clone https://github.com/PaddlePaddle/Serving
    ```
    **GPU：**
    ```
    # 启动 GPU Docker
    docker pull paddlepaddle/serving:0.8.0-cuda10.2-cudnn7-devel
    nvidia-docker run -p 9292:9292 --name test -dit paddlepaddle/serving:0.8.0-cuda10.2-cudnn7-devel bash
    nvidia-docker exec -it test bash
    git clone https://github.com/PaddlePaddle/Serving
    ```

2. 检查 Python 的版本

    使用以下命令确认是 3.6/3.7/3.8/3.9
    
        python3 --version

3. 检查 pip 的版本，确认是 20.2.2+  
    
        python3 -m ensurepip 
        python3 -m pip --version

4. 安装所需的 pip 依赖

    ```
    cd Serving
    pip3 install -r python/requirements.txt
    ```

5. 安装服务 whl 包，共有3种 client、app、server，Server 分为 CPU 和 GPU，GPU 包根据您的环境选择一种安装

    - post102 = CUDA10.2 + Cudnn7 + TensorRT6（推荐）
    - post101 = CUDA10.1 + TensorRT6
    - post112 = CUDA11.2 + TensorRT8

    ```shell
    pip3 install paddle-serving-client==0.8.2 -i https://pypi.tuna.tsinghua.edu.cn/simple
    pip3 install paddle-serving-app==0.8.2 -i https://pypi.tuna.tsinghua.edu.cn/simple
    
    # CPU Server
    pip3 install paddle-serving-server==0.8.2 -i https://pypi.tuna.tsinghua.edu.cn/simple
    
    # GPU Server，需要确认环境再选择执行哪一条，推荐使用CUDA 10.2的包
    pip3 install paddle-serving-server-gpu==0.8.2.post102 -i https://pypi.tuna.tsinghua.edu.cn/simple 
    pip3 install paddle-serving-server-gpu==0.8.2.post101 -i https://pypi.tuna.tsinghua.edu.cn/simple
    pip3 install paddle-serving-server-gpu==0.8.2.post112 -i https://pypi.tuna.tsinghua.edu.cn/simple
    ```

    默认开启国内清华镜像源来加速下载，如果您使用 HTTP 代理可以关闭(`-i https://pypi.tuna.tsinghua.edu.cn/simple`)

6. 安装 Paddle 相关 Python 库
    **当您使用`paddle_serving_client.convert`命令或者`Python Pipeline 框架`时才需要安装。**
    ```
    # CPU 环境请执行
    pip3 install paddlepaddle==2.2.2

    # GPU CUDA 10.2环境请执行
    pip3 install paddlepaddle-gpu==2.2.2
    ```
    **注意**： 如果您的 CUDA 版本不是10.2，或者您需要在 GPU 环境上使用 TensorRT，请勿直接执行上述命令，需要参考[Paddle-Inference官方文档-下载安装Linux预测库](https://paddleinference.paddlepaddle.org.cn/master/user_guides/download_lib.html#python)选择相应的 GPU 环境的 url 链接并进行安装。

7. 安装完成后的环境检查
    当以上步骤均完成后可使用命令行运行环境检查功能，自动运行 Paddle Serving 相关示例，进行环境相关配置校验。
    ```
    python3 -m paddle_serving_server.serve check
    ```
    详情请参考[环境检查文档](./Check_Env_CN.md)

8. 更多帮助信息请参考：


**第二种安装方式：使用源代码编译安装**

- 如果您只是使用 PaddleServing ，建议使用 **pip** 安装即可。
- 如果您有开发 PaddleServing 的需求，请参考 [从源码编译]()