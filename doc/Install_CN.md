# 使用Docker安装Paddle Serving

(简体中文|[English](./Install_EN.md))

**强烈建议**您在**Docker内构建**Paddle Serving，更多镜像请查看[Docker镜像列表](Docker_Images_CN.md)。

**提示-1**：本项目仅支持<mark>**Python3.6/3.7/3.8/3.9**</mark>，接下来所有的与Python/Pip相关的操作都需要选择正确的Python版本。

**提示-2**：以下示例中GPU环境均为cuda10.2-cudnn7，如果您使用Python Pipeline来部署，并需要Nvidia TensorRT来优化预测性能，请参考[支持的镜像环境和说明](#4支持的镜像环境和说明)来选择其他版本。


## 1.启动开发镜像
<mark>**同时支持使用Serving镜像和Paddle镜像，1.1和1.2章节中的操作2选1即可。**</mark> 在Paddle docker镜像上部署Serving服务需要安装额外依赖库，因此，我们直接使用Serving开发镜像。
### 1.1 Serving开发镜像（CPU/GPU 2选1）
**CPU：**
```
# 启动 CPU Docker
docker pull registry.baidubce.com/paddlepaddle/serving:0.8.0-devel
docker run -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/serving:0.8.0-devel bash
docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving
```
**GPU：**
```
# 启动 GPU Docker
docker pull registry.baidubce.com/paddlepaddle/serving:0.8.0-cuda10.2-cudnn7-devel
nvidia-docker run -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/serving:0.8.0-cuda10.2-cudnn7-devel bash
nvidia-docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving
```
### 1.2 Paddle开发镜像（CPU/GPU 2选1）
**CPU：**
```
# 启动 CPU Docker
docker pull registry.baidubce.com/paddlepaddle/paddle:2.2.2
docker run -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/paddle:2.2.2 bash
docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving

# Paddle开发镜像需要执行以下脚本增加Serving所需依赖项
bash Serving/tools/paddle_env_install.sh
```
**GPU：**
```
# 启动 GPU Docker
nvidia-docker pull registry.baidubce.com/paddlepaddle/paddle:2.2.2-gpu-cuda10.2-cudnn7
nvidia-docker run -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/paddle:2.2.2-gpu-cuda10.2-cudnn7 bash
nvidia-docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving

# Paddle开发镜像需要执行以下脚本增加Serving所需依赖项
bash Serving/tools/paddle_env_install.sh
```
## 2.安装Paddle Serving稳定wheel包

安装所需的pip依赖
```
cd Serving
pip3 install -r python/requirements.txt
```

安装服务whl包，共有3种client、app、server，Server分为CPU和GPU，GPU包根据您的环境选择一种安装
- post102 = CUDA10.2 + cuDNN7 + TensorRT6（推荐）
- post101 = CUDA10.1 + cuDNN7 + TensorRT6
- post112 = CUDA11.2 + cuDNN8 + TensorRT8

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

默认开启国内清华镜像源来加速下载，如果您使用HTTP代理可以关闭(`-i https://pypi.tuna.tsinghua.edu.cn/simple`)

如果需要使用develop分支编译的安装包，请从[下载Wheel包](./Latest_Packages_CN.md)中获取下载地址进行下载，使用`pip install`命令进行安装。如果您想自行编译，请参照[Paddle Serving编译文档](./Compile_CN.md)。

paddle-serving-server和paddle-serving-server-gpu安装包支持Centos 6/7, Ubuntu 16/18和Windows 10。

paddle-serving-client和paddle-serving-app安装包支持Linux和Windows，其中paddle-serving-client仅支持python3.6/3.7/3.8/3.9。

**如果您之前使用paddle serving 0.5.X 0.6.X的Cuda10.2环境，需要注意在0.8.0版本，paddle-serving-server-gpu==0.8.0.post102的使用Cudnn7和TensorRT6，而0.6.0.post102使用cudnn8和TensorRT7。如果0.6.0的cuda10.2用户需要升级安装，请使用paddle-serving-server-gpu==0.8.0.post1028**

## 3.安装Paddle相关Python库
**当您使用`paddle_serving_client.convert`命令或者`Python Pipeline框架`时才需要安装。**
```
# CPU环境请执行
pip3 install paddlepaddle==2.2.2 -i https://pypi.tuna.tsinghua.edu.cn/simple

# GPU CUDA 10.2环境请执行
pip3 install paddlepaddle-gpu==2.2.2 -i https://pypi.tuna.tsinghua.edu.cn/simple
```
**注意**： 如果您的Cuda版本不是10.2，或者您需要在GPU环境上使用TensorRT，请勿直接执行上述命令，需要参考[Paddle-Inference官方文档-下载安装Linux预测库](https://paddleinference.paddlepaddle.org.cn/master/user_guides/download_lib.html#python)选择相应的GPU环境的url链接并进行安装。举例假设您使用python3.6，请执行如下命令。

```
# CUDA10.1 + CUDNN7 + TensorRT6
pip3 install https://paddle-inference-lib.bj.bcebos.com/2.2.2/python/Linux/GPU/x86-64_gcc8.2_avx_mkl_cuda10.1_cudnn7.6.5_trt6.0.1.5/paddlepaddle_gpu-2.2.2.post101-cp36-cp36m-linux_x86_64.whl

# CUDA10.2 + CUDNN7 + TensorRT6, 需要注意的是此环境和Cuda10.1+Cudnn7+TensorRT6使用同一个paddle whl包
pip3 install https://paddle-inference-lib.bj.bcebos.com/2.2.2/python/Linux/GPU/x86-64_gcc8.2_avx_mkl_cuda10.1_cudnn7.6.5_trt6.0.1.5/paddlepaddle_gpu-2.2.2.post101-cp36-cp36m-linux_x86_64.whl

# CUDA10.2 + CUDNN8 + TensorRT7
pip3 install https://paddle-inference-lib.bj.bcebos.com/2.2.2/python/Linux/GPU/x86-64_gcc8.2_avx_mkl_cuda10.2_cudnn8.1.1_trt7.2.3.4/paddlepaddle_gpu-2.2.2-cp36-cp36m-linux_x86_64.whl

# CUDA11.2 + CUDNN8 + TensorRT8
pip3 install https://paddle-inference-lib.bj.bcebos.com/2.2.2/python/Linux/GPU/x86-64_gcc8.2_avx_mkl_cuda11.2_cudnn8.2.1_trt8.0.3.4/paddlepaddle_gpu-2.2.2.post112-cp36-cp36m-linux_x86_64.whl
```

例如CUDA 10.1的Python3.6用户，请选择表格当中的`cp36-cp36m`和`linux-cuda10.1-cudnn7.6-trt6-gcc8.2`对应的url，复制下来并执行
```
pip3 install https://paddle-inference-lib.bj.bcebos.com/2.2.2/python/Linux/GPU/x86-64_gcc8.2_avx_mkl_cuda10.1_cudnn7.6.5_trt6.0.1.5/paddlepaddle_gpu-2.2.2.post101-cp36-cp36m-linux_x86_64.whl
```
## 4.支持的镜像环境和说明
|  环境                         |   Serving开发镜像Tag               |    操作系统      | Paddle开发镜像Tag       |  操作系统            |
| :--------------------------: | :-------------------------------: | :-------------: | :-------------------: | :----------------: |
|  CPU                         | 0.8.0-devel                       |  Ubuntu 16.04   | 2.2.2                 | Ubuntu 18.04.       |
|  CUDA10.1 + CUDNN7             | 0.8.0-cuda10.1-cudnn7-devel       |  Ubuntu 16.04   | 无                     | 无                 |
|  CUDA10.2 + CUDNN7             | 0.8.0-cuda10.2-cudnn7-devel       |  Ubuntu 16.04   | 2.2.2-gpu-cuda10.2-cudnn7 | Ubuntu 16.04        |
|  CUDA10.2 + CUDNN8             | 0.8.0-cuda10.2-cudnn8-devel       |  Ubuntu 16.04   | 无                    |  无                 |
|  CUDA11.2 + CUDNN8             | 0.8.0-cuda11.2-cudnn8-devel       |  Ubuntu 16.04   | 2.2.2-gpu-cuda11.2-cudnn8 | Ubuntu 18.04        | 

对于**Windows 10 用户**，请参考文档[Windows平台使用Paddle Serving指导](Windows_Tutorial_CN.md)。

## 5.安装完成后的环境检查
当以上步骤均完成后可使用命令行运行环境检查功能，自动运行Paddle Serving相关示例，进行环境相关配置校验。
```
python3 -m paddle_serving_server.serve check
```
详情请参考[环境检查文档](./Check_Env_CN.md)
