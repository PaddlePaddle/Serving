(简体中文|[English](./Install_CN.md))

**强烈建议**您在**Docker内构建**Paddle Serving，请查看[如何在Docker中运行PaddleServing](RUN_IN_DOCKER_CN.md)。更多镜像请查看[Docker镜像列表](DOCKER_IMAGES_CN.md)。

**提示**：目前paddlepaddle 2.1版本的默认GPU环境是Cuda 10.2，因此GPU Docker的示例代码以Cuda 10.2为准。镜像和pip安装包也提供了其余GPU环境，用户如果使用其他环境，需要仔细甄别并选择合适的版本。

**提示**：本项目仅支持Python3.6/3.7/3.8，接下来所有的与Python/Pip相关的操作都需要选择正确的Python版本。

```
# 启动 CPU Docker
docker pull registry.baidubce.com/paddlepaddle/serving:0.6.2-devel
docker run -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/serving:0.6.2-devel bash
docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving
```
```
# 启动 GPU Docker
nvidia-docker pull registry.baidubce.com/paddlepaddle/serving:0.6.2-cuda10.2-cudnn8-devel
nvidia-docker run -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/serving:0.6.2-cuda10.2-cudnn8-devel bash
nvidia-docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving
```

安装所需的pip依赖
```
cd Serving
pip3 install -r python/requirements.txt
```

```shell
pip3 install paddle-serving-client==0.6.2
pip3 install paddle-serving-server==0.6.2 # CPU
pip3 install paddle-serving-app==0.6.2
pip3 install paddle-serving-server-gpu==0.6.2.post102 #GPU with CUDA10.2 + TensorRT7
# 其他GPU环境需要确认环境再选择执行哪一条
pip3 install paddle-serving-server-gpu==0.6.2.post101 # GPU with CUDA10.1 + TensorRT6
pip3 install paddle-serving-server-gpu==0.6.2.post11 # GPU with CUDA10.1 + TensorRT7
```

您可能需要使用国内镜像源（例如清华源, 在pip命令中添加`-i https://pypi.tuna.tsinghua.edu.cn/simple`）来加速下载。

如果需要使用develop分支编译的安装包，请从[最新安装包列表](LATEST_PACKAGES.md)中获取下载地址进行下载，使用`pip install`命令进行安装。如果您想自行编译，请参照[Paddle Serving编译文档](COMPILE_CN.md)。

paddle-serving-server和paddle-serving-server-gpu安装包支持Centos 6/7, Ubuntu 16/18和Windows 10。

paddle-serving-client和paddle-serving-app安装包支持Linux和Windows，其中paddle-serving-client仅支持python3.6/3.7/3.8。

**最新的0.6.2的版本，已经不支持Cuda 9.0和Cuda 10.0，Python已不支持2.7和3.5。**

推荐安装2.1.0及以上版本的paddle

```
# CPU环境请执行
pip3 install paddlepaddle==2.1.0

# GPU Cuda10.2环境请执行
pip3 install paddlepaddle-gpu==2.1.0
```

**注意**： 如果您的Cuda版本不是10.2，请勿直接执行上述命令，需要参考[Paddle官方文档-多版本whl包列表](https://www.paddlepaddle.org.cn/documentation/docs/zh/install/Tables.html#whl-release)

选择相应的GPU环境的url链接并进行安装，例如Cuda 10.1的Python3.6用户，请选择表格当中的`cp36-cp36m`和`cuda10.1-cudnn7-mkl-gcc8.2-avx-trt6.0.1.5`对应的url，复制下来并执行
```
pip3 install https://paddle-wheel.bj.bcebos.com/with-trt/2.1.0-gpu-cuda10.1-cudnn7-mkl-gcc8.2/paddlepaddle_gpu-2.1.0.post101-cp36-cp36m-linux_x86_64.whl
```
由于默认的`paddlepaddle-gpu==2.1.0`是Cuda 10.2，并没有联编TensorRT，因此如果需要和在`paddlepaddle-gpu`上使用TensorRT，需要在上述多版本whl包列表当中，找到`cuda10.2-cudnn8.0-trt7.1.3`，下载对应的Python版本。更多信息请参考[如何使用TensorRT?](TENSOR_RT_CN.md)。

如果是其他环境和Python版本，请在表格中找到对应的链接并用pip安装。

对于**Windows 10 用户**，请参考文档[Windows平台使用Paddle Serving指导](WINDOWS_TUTORIAL_CN.md)。
