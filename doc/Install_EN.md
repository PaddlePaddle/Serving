# Install Paddle Serving with Docker

([简体中文](./Install_CN.md)|English)

**Strongly recommend** you build **Paddle Serving** in Docker. For more images, please refer to [Docker Image List](Docker_Images_CN.md).

**Tip-1**: This project only supports <mark>**Python3.6/3.7/3.8**</mark>, all subsequent operations related to Python/Pip need to select the correct Python version.

**Tip-2**: The GPU environments in the following examples are all cuda10.2-cudnn7. If you use Python Pipeline to deploy and need Nvidia TensorRT to optimize prediction performance, please refer to [Supported Mirroring Environment and Instructions](#4.-Supported-Docker-Images-and-Instruction) to choose other versions.

## 1. Start the Docker Container
<mark>**Both Serving Dev Image and Paddle Dev Image are supported at the same time. You can choose 1 from the operation 2 in chapters 1.1 and 1.2.**</mark>Deploying the Serving service on the Paddle docker image requires the installation of additional dependency libraries. Therefore, we directly use the Serving development image.

### 1.1 Serving Dev Images (CPU/GPU 2 choose 1)
**CPU:**
```
# Start CPU Docker Container
docker pull paddlepaddle/serving:0.7.0-devel
docker run -p 9292:9292 --name test -dit paddlepaddle/serving:0.7.0-devel bash
docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving
```
**GPU:**
```
# Start GPU Docker Container
docker pull paddlepaddle/serving:0.7.0-cuda10.2-cudnn7-devel
nvidia-docker run -p 9292:9292 --name test -dit paddlepaddle/serving:0.7.0-cuda10.2-cudnn7-devel bash
nvidia-docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving
```
### 1.2 Paddle Dev Images (choose any codeblock of CPU/GPU)
**CPU:**
```
# Start CPU Docker Container
docker pull paddlepaddle/paddle:2.2.0
docker run -p 9292:9292 --name test -dit paddlepaddle/paddle:2.2.0 bash
docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving

# Paddle dev image needs to run the following script to increase the dependencies required by Serving
bash Serving/tools/paddle_env_install.sh
```
**GPU:**
```
# Start GPU Docker
docker pull paddlepaddle/paddle:2.2.0-gpu-cuda10.2-cudnn7
nvidia-docker run -p 9292:9292 --name test -dit paddlepaddle/paddle:2.2.0-gpu-cuda10.2-cudnn7 bash
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

```shell
pip3 install paddle-serving-client==0.7.0
pip3 install paddle-serving-server==0.7.0 # CPU
pip3 install paddle-serving-app==0.7.0
pip3 install paddle-serving-server-gpu==0.7.0.post102 #GPU with CUDA10.2 + Cudnn7 + TensorRT6
# Other GPU environments need to confirm the environment before choosing which one to execute
pip3 install paddle-serving-server-gpu==0.7.0.post101 # GPU with CUDA10.1 + TensorRT6
pip3 install paddle-serving-server-gpu==0.7.0.post112 # GPU with CUDA11.2 + TensorRT8
```

If you are in China, You may need to use a chinese mirror source (such as Tsinghua source, add `-i https://pypi.tuna.tsinghua.edu.cn/simple` to the pip command) to speed up the download.

If you need to use the installation package compiled by the develop branch, please download the download address from [Latest installation package list](./Latest_Packages_CN.md), and use the `pip install` command to install. If you want to compile by yourself, please refer to [Paddle Serving Compilation Document](./Compile_CN.md).

The paddle-serving-server and paddle-serving-server-gpu installation packages support Centos 6/7, Ubuntu 16/18 and Windows 10.

The paddle-serving-client and paddle-serving-app installation packages support Linux and Windows, and paddle-serving-client only supports python3.6/3.7/3.8.

**If you used the Cuda10.2 environment of paddle serving 0.5.X 0.6.X before, you need to pay attention to version 0.7.0, paddle-serving-server-gpu==0.7.0.post102 uses Cudnn7 and TensorRT6, and 0.6.0.post102 uses cudnn8 and TensorRT7. If 0.6.0 cuda10.2 users need to upgrade, please use paddle-serving-server-gpu==0.7.0.post1028**

## 3. Install Paddle related Python libraries
**You only need to install it when you use the `paddle_serving_client.convert` command or the `Python Pipeline framework`. **
```
# CPU environment please execute
pip3 install paddlepaddle==2.2.0

# GPU Cuda10.2 environment please execute
pip3 install paddlepaddle-gpu==2.2.0
```
**Note**: If your Cuda version is not 10.2 or if you want to use TensorRT(Cuda10.2 included), please do not execute the above commands directly, you need to refer to [Paddle-Inference official document-download and install the Linux prediction library](https://paddleinference.paddlepaddle.org.cn/master/user_guides/download_lib.html#python) Select the URL link of the corresponding GPU environment and install it. Assuming that you use Python3.6, please follow the codeblock.

```
# Cuda10.1 + Cudnn7 + TensorRT6
pip3 install https://paddle-inference-lib.bj.bcebos.com/2.2.0/python/Linux/GPU/x86-64_gcc8.2_avx_mkl_cuda10.1_cudnn7.6.5_trt6.0.1.5/paddlepaddle_gpu-2.2.0.post101-cp36-cp36m-linux_x86_64.whl

# Cuda10.2 + Cudnn7 + TensorRT6, Attenton that the paddle whl for this env is same to that of Cuda10.1 + Cudnn7 + TensorRT6
pip3 install https://paddle-inference-lib.bj.bcebos.com/2.2.0/python/Linux/GPU/x86-64_gcc8.2_avx_mkl_cuda10.1_cudnn7.6.5_trt6.0.1.5/paddlepaddle_gpu-2.2.0.post101-cp36-cp36m-linux_x86_64.whl

# Cuda10.2 + Cudnn8 + TensorRT7
pip3 install https://paddle-inference-lib.bj.bcebos.com/2.2.0/python/Linux/GPU/x86-64_gcc8.2_avx_mkl_cuda10.2_cudnn8.1.1_trt7.2.3.4/paddlepaddle_gpu-2.2.0-cp36-cp36m-linux_x86_64.whl

# Cuda11.2 + Cudnn8 + TensorRT8
pip3 install https://paddle-inference-lib.bj.bcebos.com/2.2.0/python/Linux/GPU/x86-64_gcc8.2_avx_mkl_cuda11.2_cudnn8.2.1_trt8.0.3.4/paddlepaddle_gpu-2.2.0.post112-cp36-cp36m-linux_x86_64.whl
```

## 4. Supported Docker Images and Instruction


| Environment | Serving Development Image Tag | Operating System | Paddle Development Image Tag | Operating System |
| :--------------------------: | :-------------------------------: | :-------------: | :-------------------: | :----------------: |
|  CPU                         | 0.7.0-devel                       |  Ubuntu 16.04   | 2.2.0                 | Ubuntu 18.04.       |
|  Cuda10.1+Cudnn7             | 0.7.0-cuda10.1-cudnn7-devel       |  Ubuntu 16.04   | 无                     | 无                 |
|  Cuda10.2+Cudnn7             | 0.7.0-cuda10.2-cudnn7-devel       |  Ubuntu 16.04   | 2.2.0-gpu-cuda10.2-cudnn7 | Ubuntu 16.04        |
|  Cuda10.2+Cudnn8             | 0.7.0-cuda10.2-cudnn8-devel       |  Ubuntu 16.04   | 无                    |  无                 |
|  Cuda11.2+Cudnn8             | 0.7.0-cuda11.2-cudnn8-devel       |  Ubuntu 16.04   | 2.2.0-gpu-cuda11.2-cudnn8 | Ubuntu 18.04        | 

For **Windows 10 users**, please refer to the document [Paddle Serving Guide for Windows Platform](Windows_Tutorial_CN.md).
