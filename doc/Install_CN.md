# 使用Docker安装Paddle Serving

(简体中文|[English](./Install_EN.md))

**强烈建议**您在**Docker内构建**Paddle Serving，更多镜像请查看[Docker镜像列表](Docker_Images_CN.md)。

**提示-1**：本项目仅支持<mark>**Python3.6/3.7/3.8/3.9**</mark>，接下来所有的与Python/Pip相关的操作都需要选择正确的Python版本。

**提示-2**：以下示例中GPU环境均为cuda11.2-cudnn8，如果您使用Python Pipeline来部署，并需要Nvidia TensorRT来优化预测性能，请参考[支持的镜像环境和说明](#4支持的镜像环境和说明)来选择其他版本。


## 1.启动开发镜像
<mark>**同时支持使用Serving镜像和Paddle镜像，1.1和1.2章节中的操作2选1即可。**</mark> 在Paddle docker镜像上部署Serving服务需要安装额外依赖库，因此，我们直接使用Serving开发镜像。
### 1.1 Serving开发镜像（CPU/GPU 2选1）
**CPU：**
```
# 启动 CPU Docker
docker pull registry.baidubce.com/paddlepaddle/serving:0.9.0-devel
docker run -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/serving:0.9.0-devel bash
docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving
```
**GPU：**
```
# 启动 GPU Docker
docker pull registry.baidubce.com/paddlepaddle/serving:0.9.0-cuda11.2-cudnn8-devel
nvidia-docker run -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/serving:0.9.0-cuda11.2-cudnn8-devel bash
nvidia-docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving
```
### 1.2 Paddle开发镜像（CPU/GPU 2选1）
**CPU：**
```
# 启动 CPU Docker
nvidia-docker pull registry.baidubce.com/paddlepaddle/paddle:2.3.0
docker run -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/paddle:2.3.0 bash
docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving

# Paddle开发镜像需要执行以下脚本增加Serving所需依赖项
bash Serving/tools/paddle_env_install.sh
```
**GPU：**
```
# 启动 GPU Docker

nvidia-docker pull registry.baidubce.com/paddlepaddle/paddle:2.3.0-gpu-cuda11.2-cudnn8
nvidia-docker run -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/paddle:2.3.0-gpu-cuda11.2-cudnn8 bash
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
- post112 = CUDA11.2 + cuDNN8 + TensorRT8（推荐）
- post102 = CUDA10.2 + cuDNN8 + TensorRT7
- post101 = CUDA10.1 + cuDNN7 + TensorRT6


```shell
pip3 install paddle-serving-client==0.9.0 -i https://pypi.tuna.tsinghua.edu.cn/simple
pip3 install paddle-serving-app==0.9.0 -i https://pypi.tuna.tsinghua.edu.cn/simple

# CPU Server
pip3 install paddle-serving-server==0.9.0 -i https://pypi.tuna.tsinghua.edu.cn/simple

# GPU Server，需要确认环境再选择执行哪一条，推荐使用CUDA 11.2的包
pip3 install paddle-serving-server-gpu==0.9.0.post112 -i https://pypi.tuna.tsinghua.edu.cn/simple
pip3 install paddle-serving-server-gpu==0.9.0.post101 -i https://pypi.tuna.tsinghua.edu.cn/simple
pip3 install paddle-serving-server-gpu==0.9.0.post102 -i https://pypi.tuna.tsinghua.edu.cn/simple 
```

默认开启国内清华镜像源来加速下载，如果您使用HTTP代理可以关闭(`-i https://pypi.tuna.tsinghua.edu.cn/simple`)

如果需要使用develop分支编译的安装包，请从[下载Wheel包](./Latest_Packages_CN.md)中获取下载地址进行下载，使用`pip install`命令进行安装。如果您想自行编译，请参照[Paddle Serving编译文档](./Compile_CN.md)。

paddle-serving-server和paddle-serving-server-gpu安装包支持Centos 6/7, Ubuntu 16/18和Windows 10。

paddle-serving-client和paddle-serving-app安装包支持Linux和Windows，其中paddle-serving-client仅支持python3.6/3.7/3.8/3.9。

## 3.安装Paddle相关Python库

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

## 4.支持的镜像环境和说明
|  环境                         |   Serving开发镜像Tag               |    操作系统      | Paddle开发镜像Tag       |  操作系统            |
| :--------------------------: | :-------------------------------: | :-------------: | :-------------------: | :----------------: |
|  CPU                         | 0.9.0-devel                       |  Ubuntu 16.04   | 2.3.0                | Ubuntu 18.04.       |
|  CUDA10.1 + CUDNN7           | 0.9.0-cuda10.1-cudnn7-devel       |  Ubuntu 16.04   | 无                   | 无                 |
|  CUDA10.2 + CUDNN8           | 0.9.0-cuda10.2-cudnn8-devel       |  Ubuntu 16.04   | 无                   | Ubuntu 18.04   |
|  CUDA11.2 + CUDNN8           | 0.9.0-cuda11.2-cudnn8-devel       |  Ubuntu 16.04   | 2.3.0-gpu-cuda11.2-cudnn8 | Ubuntu 18.04   | 

对于**Windows 10 用户**，请参考文档[Windows平台使用Paddle Serving指导](Windows_Tutorial_CN.md)。

## 5.安装完成后的环境检查
当以上步骤均完成后可使用命令行运行环境检查功能，自动运行Paddle Serving相关示例，进行环境相关配置校验。
```
python3 -m paddle_serving_server.serve check
```
详情请参考[环境检查文档](./Check_Env_CN.md)
