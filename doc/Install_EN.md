# Install Paddle Serving with Docker

([简体中文](./Install_CN.md)|English)

**Strongly recommend** you build **Paddle Serving** in Docker. For more images, please refer to [Docker Image List](Docker_Images_CN.md).

**Tip-1**: This project only supports <mark>**Python3.6/3.7/3.8/3.9**</mark>, all subsequent operations related to Python/Pip need to select the correct Python version.

**Tip-2**: The GPU environments in the following examples are all cuda11.2-cudnn8. If you use Python Pipeline to deploy and need Nvidia TensorRT to optimize prediction performance, please refer to [Supported Mirroring Environment and Instructions](#4.-Supported-Docker-Images-and-Instruction) to choose other versions.

## 1. Start the Docker Container
<mark>**Both Serving Dev Image and Paddle Dev Image are supported at the same time. You can choose 1 from the operation 2 in chapters 1.1 and 1.2.**</mark>Deploying the Serving service on the Paddle docker image requires the installation of additional dependency libraries. Therefore, we directly use the Serving development image.

### 1.1 Serving Dev Images (CPU/GPU 2 choose 1)
**CPU:**
```
# Start CPU Docker Container
docker pull registry.baidubce.com/paddlepaddle/serving:0.9.0-devel
docker run -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/serving:0.9.0-devel bash
docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving
```
**GPU:**
```
# Start GPU Docker Container
docker pull registry.baidubce.com/paddlepaddle/serving:0.9.0-cuda11.2-cudnn7-devel
nvidia-docker run -p 9292:9292 --name test -dit docker pull registry.baidubce.com/paddlepaddle/serving:0.9.0-cuda11.2-cudnn7-devel bash
nvidia-docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving
```
### 1.2 Paddle Dev Images (choose any codeblock of CPU/GPU)
**CPU:**
```
# Start CPU Docker Container
docker pull registry.baidubce.com/paddlepaddle/paddle:2.3.0
docker run -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/paddle:2.3.0 bash
docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving

# Paddle dev image needs to run the following script to increase the dependencies required by Serving
bash Serving/tools/paddle_env_install.sh
```
**GPU:**
```
# Start GPU Docker
nvidia-docker pull registry.baidubce.com/paddlepaddle/paddle:2.3.0-gpu-cuda11.2-cudnn8
nvidia-docker run -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/paddle:2.3.0-gpu-cuda11.2-cudnn8 bash
nvidia-docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving

# Paddle development image needs to execute the following script to increase the dependencies required by Serving
bash Serving/tools/paddle_env_install.sh
```

## 2. Install Paddle Serving stable wheel packages

Install the required pip dependencies
```
cd Serving
pip3 install -r python/requirements.txt
```

Install the service whl package. There are three types of client, app and server. The server is divided into CPU and GPU. Choose one installation according to the environment. 
- post112 = CUDA11.2 + cuDNN8 + TensorRT8（Recommanded）
- post101 = CUDA10.1 + cuDNN7 + TensorRT6
- post102 = CUDA10.2 + cuDNN8 + TensorRT7


```shell
pip3 install paddle-serving-client==0.9.0 -i https://pypi.tuna.tsinghua.edu.cn/simple
pip3 install paddle-serving-app==0.9.0 -i https://pypi.tuna.tsinghua.edu.cn/simple

# CPU Server
pip3 install paddle-serving-server==0.9.0 -i https://pypi.tuna.tsinghua.edu.cn/simple

# GPU environments need to confirm the environment before choosing which one to execute
pip3 install paddle-serving-server-gpu==0.9.0.post112 -i https://pypi.tuna.tsinghua.edu.cn/simple
pip3 install paddle-serving-server-gpu==0.9.0.post102 -i https://pypi.tuna.tsinghua.edu.cn/simple 
pip3 install paddle-serving-server-gpu==0.9.0.post101 -i https://pypi.tuna.tsinghua.edu.cn/simple
```

By default, the domestic Tsinghua mirror source is turned on to speed up the download. If you use a proxy, you can turn it off（`-i https://pypi.tuna.tsinghua.edu.cn/simple`).

If you need to use the installation package compiled by the develop branch, please download the download address from [Download wheel packages](./Latest_Packages_EN.md), and use the `pip install` command to install. If you want to compile by yourself, please refer to [Paddle Serving Compilation Document](./Compile_CN.md).

The paddle-serving-server and paddle-serving-server-gpu installation packages support Centos 6/7, Ubuntu 16/18 and Windows 10.

The paddle-serving-client and paddle-serving-app installation packages support Linux and Windows, and paddle-serving-client only supports python3.6/3.7/3.8/3.9.

## 3. Install Paddle related Python libraries
**You only need to install it when you use the `paddle_serving_client.convert` command or the `Python Pipeline framework`. **
```
# CPU environment please execute
pip3 install paddlepaddle==2.3.0 -i https://pypi.tuna.tsinghua.edu.cn/simple

# GPU CUDA 11.2 environment please execute
pip3 install paddlepaddle-gpu==2.3.0.post112 -i https://pypi.tuna.tsinghua.edu.cn/simple
```
**Note**: If you want to use other versions, please do not execute the above commands directly, you need to refer to [Paddle-Inference official document-download and install the Linux prediction library](https://paddleinference.paddlepaddle.org.cn/master/user_guides/download_lib.html#python) Select the URL link of the corresponding GPU environment and install it. Assuming that you use Python3.6, please follow the codeblock.

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

## 4. Supported Docker Images and Instruction


| Environment | Serving Development Image Tag | Operating System | Paddle Development Image Tag | Operating System |
| :--------------------------: | :-------------------------------: | :-------------: | :-------------------: | :----------------: |
|  CPU                         | 0.9.0-devel                       |  Ubuntu 16.04   | 2.3.0                | Ubuntu 18.04.       |
|  CUDA10.1 + CUDNN7           | 0.9.0-cuda10.1-cudnn7-devel       |  Ubuntu 16.04   | 无                   | 无                 |
|  CUDA10.2 + CUDNN8           | 0.9.0-cuda10.2-cudnn8-devel       |  Ubuntu 16.04   | 无                   | Ubuntu 18.04   |
|  CUDA11.2 + CUDNN8           | 0.9.0-cuda11.2-cudnn8-devel       |  Ubuntu 16.04   | 2.3.0-gpu-cuda11.2-cudnn8 | Ubuntu 18.04   |

For **Windows 10 users**, please refer to the document [Paddle Serving Guide for Windows Platform](Windows_Tutorial_CN.md).

## 5.Installation Check
When the above steps are completed, you can use the command line to run the environment check function to automatically run the Paddle Serving related examples to verify the environment-related configuration.
```
python3 -m paddle_serving_server.serve check
```
For more information, please see[Installation Check](./Check_Env_CN.md)
