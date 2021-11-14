# Install Paddle Serving with Docker

([简体中文](Install_CN.md)|English)

We **highly recommend** you to **run Paddle Serving in Docker**, please visit [Run in Docker](Run_In_Docker_EN.md). See the [document](Docker_Images_EN.md) for more docker images.

**Attention:**: Currently, the default GPU environment of paddlepaddle 2.1 is Cuda 10.2, so the sample code of GPU Docker is based on Cuda 10.2. We also provides docker images and whl packages for other GPU environments. If users use other environments, they need to carefully check and select the appropriate version.

**Attention:** the following so-called 'python' or 'pip' stands for one of Python 3.6/3.7/3.8.

```
# Run CPU Docker
docker pull registry.baidubce.com/paddlepaddle/serving:0.6.0-devel
docker run -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/serving:0.6.0-devel bash
docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving
```
```
# Run GPU Docker
nvidia-docker pull registry.baidubce.com/paddlepaddle/serving:0.6.0-cuda10.2-cudnn8-devel
nvidia-docker run -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/serving:0.6.0-cuda10.2-cudnn8-devel bash
nvidia-docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving
```
install python dependencies
```
cd Serving
pip install -r python/requirements.txt
```

```shell
pip install paddle-serving-client==0.6.0
pip install paddle-serving-server==0.6.0 # CPU
pip install paddle-serving-app==0.6.0
pip install paddle-serving-server-gpu==0.6.0.post102 #GPU with CUDA10.2 + TensorRT7
# DO NOT RUN ALL COMMANDS! check your GPU env and select the right one
pip install paddle-serving-server-gpu==0.6.0.post101 # GPU with CUDA10.1 + TensorRT6
pip install paddle-serving-server-gpu==0.6.0.post11 # GPU with CUDA10.1 + TensorRT7


You may need to use a domestic mirror source (in China, you can use the Tsinghua mirror source, add `-i https://pypi.tuna.tsinghua.edu.cn/simple` to pip command) to speed up the download.

If you need install modules compiled with develop branch, please download packages from [latest packages list](Latest_Package_CN.md) and install with `pip install` command. If you want to compile by yourself, please refer to [How to compile Paddle Serving?](Compile_EN.md)

Packages of paddle-serving-server and paddle-serving-server-gpu support Centos 6/7, Ubuntu 16/18, Windows 10.

Packages of paddle-serving-client and paddle-serving-app support Linux and Windows, but paddle-serving-client only support python3.6/3.7/3.8.

**For latest version, Cuda 9.0 or Cuda 10.0 are no longer supported, Python2.7/3.5 is no longer supported.**

Recommended to install paddle >= 2.1.0


```
# CPU users, please run
pip install paddlepaddle==2.1.0

# GPU Cuda10.2 please run
pip install paddlepaddle-gpu==2.1.0 
```

**Note**: If your Cuda version is not 10.2, please do not execute the above commands directly, you need to refer to [Paddle official documentation-multi-version whl package list
](https://www.paddlepaddle.org.cn/documentation/docs/en/install/Tables_en.html#multi-version-whl-package-list-release)

Select the url link of the corresponding GPU environment and install it. For example, for Python3.6 users of Cuda 10.1, please select `cp36-cp36m` and
The url corresponding to `cuda10.1-cudnn7-mkl-gcc8.2-avx-trt6.0.1.5`, copy it and run
```
pip install https://paddle-wheel.bj.bcebos.com/with-trt/2.1.0-gpu-cuda10.1-cudnn7-mkl-gcc8.2/paddlepaddle_gpu-2.1.0.post101-cp36-cp36m-linux_x86_64.whl
```

the default `paddlepaddle-gpu==2.1.0` is Cuda 10.2 with no TensorRT. If you want to install PaddlePaddle with TensorRT. please also check the documentation-multi-version whl package list and find key word `cuda10.2-cudnn8.0-trt7.1.3`. 

If it is other environment and Python version, please find the corresponding link in the table and install it with pip.

For **Windows Users**, please read the document [Paddle Serving for Windows Users](Windows_Tutorial_EN.md)

<h2 align="center">Quick Start Example</h2>

This quick start example is mainly for those users who already have a model to deploy, and we also provide a model that can be used for deployment. in case if you want to know how to complete the process from offline training to online service, please refer to the AiStudio tutorial above.
