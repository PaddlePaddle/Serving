# Install Paddle Serving with Docker

([简体中文](./Install_CN.md)|English)

- [1.Use devel docker](#1)
    - [Serving devel images](#1.1)
        - [CPU images](#1.1.1)
        - [GPU images](#1.1.2)
        - [ARM & XPU images](#1.1.3)
    - [Paddle devel images](#1.2)
        - [CPU images](#1.2.1)
        - [GPU images](#1.2.2)
- [2.Install Wheel Packages](#2)
    - [Online Install](#2.1)
    - [Offline Install](#2.2)
    - [ARM & XPU Install](#2.3)
- [3.Installation Check](#3)

**Strongly recommend** you build **Paddle Serving** in Docker. For more images, please refer to [Docker Image List](Docker_Images_CN.md).

**Tip-1**: This project only supports <mark>**Python3.6/3.7/3.8/3.9**</mark>, all subsequent operations related to Python/Pip need to select the correct Python version.

**Tip-2**: The GPU environments in the following examples are all cuda11.2-cudnn8. If you use Python Pipeline to deploy and need Nvidia TensorRT to optimize prediction performance, please refer to [Supported Mirroring Environment and Instructions](#4.-Supported-Docker-Images-and-Instruction) to choose other versions.

<a name="1"></a>

## 1.Use devel docker
<mark>**Both Serving Dev Image and Paddle Dev Image are supported at the same time. You can choose 1 from the operation 2 in chapters 1.1 and 1.2.**</mark>Deploying the Serving service on the Paddle docker image requires the installation of additional dependency libraries. Therefore, we directly use the Serving development image.

| Environment | Serving Development Image Tag | Operating System | Paddle Development Image Tag | Operating System |
| :--------------------------: | :-------------------------------: | :-------------: | :-------------------: | :----------------: |
|  CPU                         | 0.9.0-devel                       |  Ubuntu 16    | 2.3.0                    | Ubuntu 18       |
|  CUDA10.1 + cuDNN 7           | 0.9.0-cuda10.1-cudnn7-devel       |  Ubuntu 16   | None                       | None                 |
|  CUDA10.2 + cuDNN 7           | 0.9.0-cuda10.2-cudnn7-devel       |  Ubuntu 16   | 2.3.0-gpu-cuda10.2-cudnn7 | Ubuntu 18
|  CUDA10.2 + cuDNN 8           | 0.9.0-cuda10.2-cudnn8-devel       |  Ubuntu 16   | None                   | None |
|  CUDA11.2 + cuDNN 8           | 0.9.0-cuda11.2-cudnn8-devel       |  Ubuntu 16   | 2.3.0-gpu-cuda11.2-cudnn8 | Ubuntu 18   | 
|  ARM + XPU                    | xpu-arm                           |  CentOS 8.3  | None                         | None           |

For **Windows 10 users**, please refer to the document [Paddle Serving Guide for Windows Platform](Windows_Tutorial_CN.md).


<a name="1.1"></a>

### 1.1 Serving Devel Images (CPU/GPU 2 choose 1)

<a name="1.1.1"></a>

**CPU:**
```
# Start CPU Docker Container
docker pull registry.baidubce.com/paddlepaddle/serving:0.9.0-devel
docker run -p 9292:9292 --name test_cpu -dit registry.baidubce.com/paddlepaddle/serving:0.9.0-devel bash
docker exec -it test_cpu bash
git clone https://github.com/PaddlePaddle/Serving
```

<a name="1.1.2"></a>

**GPU:**
```
# Start GPU Docker Container
docker pull registry.baidubce.com/paddlepaddle/serving:0.9.0-cuda11.2-cudnn7-devel
nvidia-docker run -p 9292:9292 --name test_gpu -dit docker pull registry.baidubce.com/paddlepaddle/serving:0.9.0-cuda11.2-cudnn7-devel bash
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

### 1.2 Paddle Devel Images (choose any codeblock of CPU/GPU)

<a name="1.2.1"></a>

**CPU:**
```shell
# Start CPU Docker Container
docker pull registry.baidubce.com/paddlepaddle/paddle:2.3.0
docker run -p 9292:9292 --name test_cpu -dit registry.baidubce.com/paddlepaddle/paddle:2.3.0 bash
docker exec -it test_cpu bash
git clone https://github.com/PaddlePaddle/Serving

### Paddle dev image needs to run the following script to increase the dependencies required by Serving
bash Serving/tools/paddle_env_install.sh
```

<a name="1.2.2"></a>

**GPU:**

```shell
### Start GPU Docker
nvidia-docker pull registry.baidubce.com/paddlepaddle/paddle:2.3.0-gpu-cuda11.2-cudnn8
nvidia-docker run -p 9292:9292 --name test_gpu -dit registry.baidubce.com/paddlepaddle/paddle:2.3.0-gpu-cuda11.2-cudnn8 bash
nvidia-docker exec -it test_gpu bash
git clone https://github.com/PaddlePaddle/Serving

### Paddle development image needs to execute the following script to increase the dependencies required by Serving
bash Serving/tools/paddle_env_install.sh
```

<a name="2"></a>

## 2. Install wheel packages

Install the required pip dependencies
```
cd Serving
pip3 install -r python/requirements.txt
```

Install the service whl package. There are three types of client, app and server. The server is divided into CPU and GPU. Choose one installation according to the environment. 
- post112 = CUDA11.2 + cuDNN8 + TensorRT8（Recommanded）
- post101 = CUDA10.1 + cuDNN7 + TensorRT6
- post102 = CUDA10.2 + cuDNN7 + TensorRT6 (The same with paddle docker images)
- post1028 = CUDA10.2 + cuDNN8 + TensorRT7

<a name="2.1"></a>

### 2.1 Online Install
Online installation uses `pypi` to download and install.

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
<a name="2.2"></a>

### 2.2 Offline Install
Offline installation is to download all Paddle and Serving packages and dependent libraries, and install them in a no-network or weak-network environment.

**1.Install offline wheel packages**

The independent dependencies of the Serving and Paddle Wheel packages are downloaded by four links.

```
wget https://paddle-serving.bj.bcebos.com/offline_wheels/0.9.0/py36_offline_whl_packages.tar
wget https://paddle-serving.bj.bcebos.com/offline_wheels/0.9.0/py37_offline_whl_packages.tar
wget https://paddle-serving.bj.bcebos.com/offline_wheels/0.9.0/py38_offline_whl_packages.tar
wget https://paddle-serving.bj.bcebos.com/offline_wheels/0.9.0/py39_offline_whl_packages.tar
```

The Serving and Paddle Wheel packages can be installed locally by running the `install.py` script. The parameter list for the `install.py` script is as follows:
```
python3 install.py
  --python_version : Python version for installing wheels, one of [py36, py37, py38, py39], py37 default.
  --device : Type of devices, one of [cpu, gpu], cpu default.
  --cuda_version : CUDA version for GPU, one of [101, 102, 112, empty], empty default.
  --serving_version : Verson of Serving, one of [0.9.0, no_install], 0.9.0 default.
  --paddle_version Verson of Paddle, one of [2.3.0, no_install], 2.3.0 default.
```

**2.Specify the `SERVING_BIN` path in the environment variable**
After completing step 1 of the installation, you can ignore this step if you only use the python pipeline mode.

If you use C++ Serving to start the service using the command line, the example is as follows. Then you need to export the environment variable `SERVING_BIN` in the command line window or service launcher, and use the local serving binary to run the service.
C++ Serving command line start service example:
```
python3 -m paddle_serving_server.serve --model serving_model --thread 10 --port 9292 --gpu_ids 0,1,2
```
Since the binary package for all versions has 20 GB, it is very large. Therefore, multiple versions of download links are provided. Manually `wget` downloads the specified version to the `serving_bin` directory, decompresses it and exports it to the environment variable.

- cpu-avx-mkl: https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-cpu-avx-mkl-0.9.0.tar.gz
- cpu-avx-openblas: https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-cpu-avx-openblas-0.9.0.tar.gz
- cpu-noavx-openblas: https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-cpu-noavx-openblas-0.9.0.tar.gz
- cuda10.1-cudnn7-TensorRT6: https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-gpu-101-0.9.0.tar.gz
- cuda10.2-cudnn7-TensorRT6: https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-gpu-102-0.9.0.tar.gz
- cuda10.2-cudnn8-TensorRT7: https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-gpu-1028-0.9.0.tar.gz
- cuda11.2-cudnn8-TensorRT8: https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-gpu-112-0.9.0.tar.gz

Taking GPU CUDA 10.2 as an example, set the environment variables on the command line or in the launcher as follows:

**3.Run `install.py` to install wheel packages**

1. Install the py38 version GPU wheel package of Serving and Paddle at the same time:
```
python3 install.py --cuda_version="102" --python_version="py38" --device="GPU" --serving_version="0.9.0" --paddle_version="2.3.0"
```

2.Only install the py39 version of the Serving CPU wheel package, set `--paddle_version="no_install"` to not install the Paddle prediction library, set `--device="cpu"` to indicate the cpu version
```
python3 install.py --cuda_version="" --python_version="py39" --device="cpu" --serving_version="0.9.0" --paddle_version="no_install"
```
3. Install only the GPU wheel package of Paddle's py36 version `cuda=11.2`
```
python3 install.py --cuda_version="112" --python_version="py36" --device="GPU" --serving_version="no_install" --paddle_version="2.3.0"
```

<a name="2.3"></a>

### 2.3 ARM & XPU Install

Since there are few users using ARM and XPU, the Wheel for installing this environment is provided separately as follows, among which `paddle_serving_client` only provides the `py36` version, if you need other versions, please contact us.
```
pip3.6 install https://paddle-serving.bj.bcebos.com/test-dev/whl/arm/paddle_serving_app-0.9.0-py3-none-any.whl
pip3.6 install https://paddle-serving.bj.bcebos.com/test-dev/whl/arm/paddle_serving_client-0.9.0-cp36-none-any.whl
pip3.6 install https://paddle-serving.bj.bcebos.com/test-dev/whl/arm/paddle_serving_server_xpu-0.9.0.post2-py3-none-any.whl
```

Download binary package address:
```
wget https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-xpu-aarch64-0.9.0.tar.gz
```

<a name="3"></a>

## 3.Installation Check
When the above steps are completed, you can use the command line to run the environment check function to automatically run the Paddle Serving related examples to verify the environment-related configuration.
```
python3 -m paddle_serving_server.serve check
```
For more information, please see[Installation Check](./Check_Env_CN.md)
