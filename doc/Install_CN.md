# 使用Docker安装Paddle Serving

(简体中文|[English](./Install_EN.md))

**强烈建议**您在**Docker内构建**Paddle Serving，请查看[如何在Docker中运行PaddleServing](Run_In_Docker_CN.md)。更多镜像请查看[Docker镜像列表](Docker_Images_CN.md)。

**提示-1**：本项目仅支持<mark>**Python3.6/3.7/3.8**</mark>，接下来所有的与Python/Pip相关的操作都需要选择正确的Python版本。

**提示-2**：以下示例中GPU环境均为cuda10.2-cudnn7，如果您使用Python Pipeline来部署，并需要Nvidia TensorRT来优化预测性能，请参考[支持的镜像环境和说明](#4支持的镜像环境和说明)来选择其他版本。


## 1.启动开发镜像
<mark>**同时支持使用Serving镜像和Paddle镜像，1.1和1.2章节中的操作2选1即可。**</mark>
### 1.1 Serving开发镜像（CPU/GPU 2选1）
**CPU：**
```
# 启动 CPU Docker
docker pull paddlepaddle/serving:0.7.0-devel
docker run -p 9292:9292 --name test -dit paddlepaddle/serving:0.7.0-devel bash
docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving
```
**GPU：**
```
# 启动 GPU Docker
docker pull paddlepaddle/serving:0.7.0-cuda10.2-cudnn7-devel
nvidia-docker run -p 9292:9292 --name test -dit paddlepaddle/serving:0.7.0-cuda10.2-cudnn7-devel bash
nvidia-docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving
```
### 1.2 Paddle开发镜像（CPU/GPU 2选1）
**CPU：**
```
# 启动 CPU Docker
docker pull paddlepaddle/paddle:2.2.0
docker run -p 9292:9292 --name test -dit paddlepaddle/paddle:2.2.0 bash
docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving

# Paddle开发镜像需要执行以下脚本增加Serving所需依赖项
bash Serving/tools/paddle_env_install.sh
```
**GPU：**
```
# 启动 GPU Docker
docker pull paddlepaddle/paddle:2.2.0-cuda10.2-cudnn7
nvidia-docker run -p 9292:9292 --name test -dit paddlepaddle/paddle:2.2.0-cuda10.2-cudnn7 bash
nvidia-docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving

# Paddle开发镜像需要执行以下脚本增加Serving所需依赖项
bash Serving/tools/paddle_env_install.sh
```
## 2.安装Paddle Serving相关Python库

安装所需的pip依赖
```
cd Serving
pip3 install -r python/requirements.txt
```

```shell
pip3 install paddle-serving-client==0.7.0
pip3 install paddle-serving-server==0.7.0 # CPU
pip3 install paddle-serving-app==0.7.0
pip3 install paddle-serving-server-gpu==0.7.0.post102 #GPU with CUDA10.2 + TensorRT6
# 其他GPU环境需要确认环境再选择执行哪一条
pip3 install paddle-serving-server-gpu==0.7.0.post101 # GPU with CUDA10.1 + TensorRT6
pip3 install paddle-serving-server-gpu==0.7.0.post112 # GPU with CUDA11.2 + TensorRT8
```

您可能需要使用国内镜像源（例如清华源, 在pip命令中添加`-i https://pypi.tuna.tsinghua.edu.cn/simple`）来加速下载。

如果需要使用develop分支编译的安装包，请从[最新安装包列表](./Latest_Packages_CN.md)中获取下载地址进行下载，使用`pip install`命令进行安装。如果您想自行编译，请参照[Paddle Serving编译文档](./Compile_CN.md)。

paddle-serving-server和paddle-serving-server-gpu安装包支持Centos 6/7, Ubuntu 16/18和Windows 10。

paddle-serving-client和paddle-serving-app安装包支持Linux和Windows，其中paddle-serving-client仅支持python3.6/3.7/3.8。

## 3.安装Paddle相关Python库
**当您使用`paddle_serving_client.convert`命令或者`Python Pipeline框架`时才需要安装。**
```
# CPU环境请执行
pip3 install paddlepaddle==2.2.0

# GPU Cuda10.2环境请执行
pip3 install paddlepaddle-gpu==2.2.0
```
**注意**： 如果您的Cuda版本不是10.2，请勿直接执行上述命令，需要参考[Paddle-Inference官方文档-下载安装Linux预测库](https://paddleinference.paddlepaddle.org.cn/master/user_guides/download_lib.html#python)选择相应的GPU环境的url链接并进行安装。

例如Cuda 10.1的Python3.6用户，请选择表格当中的`cp36-cp36m`和`linux-cuda10.1-cudnn7.6-trt6-gcc8.2`对应的url，复制下来并执行
```
pip3 install https://paddle-inference-lib.bj.bcebos.com/2.2.0/python/Linux/GPU/x86-64_gcc8.2_avx_mkl_cuda10.1_cudnn7.6.5_trt6.0.1.5/paddlepaddle_gpu-2.2.0.post101-cp36-cp36m-linux_x86_64.whl
```
## 4.支持的镜像环境和说明
|  环境                         |   Serving开发镜像Tag               |    操作系统      | Paddle开发镜像Tag       |  操作系统            |
| :--------------------------: | :-------------------------------: | :-------------: | :-------------------: | :----------------: |
|  CPU                         | 0.7.0-devel                       |  Ubuntu 16.04   | 2.2.0                 | Ubuntu 18.04.       |
|  Cuda10.1+Cudnn7             | 0.7.0-cuda10.1-cudnn7-devel       |  Ubuntu 16.04   | 无                     | 无                 |
|  Cuda10.2+Cudnn7             | 0.7.0-cuda10.2-cudnn7-devel       |  Ubuntu 16.04   | 2.2.0-cuda10.2-cudnn7 | Ubuntu 16.04        |
|  Cuda10.2+Cudnn8             | 0.7.0-cuda10.2-cudnn8-devel       |  Ubuntu 16.04   | 无                    |  无                 |
|  Cuda11.2+Cudnn8             | 0.7.0-cuda11.2-cudnn8-devel       |  Ubuntu 16.04   | 2.2.0-cuda11.2-cudnn8 | Ubuntu 18.04        | 

对于**Windows 10 用户**，请参考文档[Windows平台使用Paddle Serving指导](Windows_Tutorial_CN.md)。
