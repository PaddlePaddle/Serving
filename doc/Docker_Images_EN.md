# Docker Images

([简体中文](Docker_Images_CN.md)|English)

This document maintains a list of docker images provided by Paddle Serving.

## Get docker image

You can get images in two ways:

1. Pull image directly from dockerhub or `registry.baidubce.com ` through TAG:

   ```shell
   docker pull registry.baidubce.com/paddlepaddle/serving:<TAG> 
   ```

2. Building image based on dockerfile

   Create a new folder and copy Dockerfile to this folder, and run the following command:

   ```shell
   docker build -f tools/${DOCKERFILE} -t <image-name>:<images-tag> .
   ```



## Image description

If you want to customize your Serving based on source code, use the version with the suffix - devel.

**cuda10.1-cudnn7-gcc54 image is not ready, you should run from dockerfile if you need it.**

If you need to develop and compile based on the source code, please use the version with the suffix -devel.
**In the TAG column, 0.8.0 can also be replaced with the corresponding version number, such as 0.5.0/0.4.1, etc., but it should be noted that some development environments only increase with a certain version iteration, so not all environments All have the corresponding version number can be used.**

|                         Description                         |   OS    |             TAG              |                          Dockerfile                          |
| :----------------------------------------------------------: | :-----: | :--------------------------: | :----------------------------------------------------------: |
|                       CPU development                        | Ubuntu16 |         0.8.0-devel         |        [Dockerfile.devel](../tools/Dockerfile.devel)         |
|              GPU (cuda10.1-cudnn7-tensorRT6-gcc54) development               | Ubuntu16 | 0.8.0-cuda10.1-cudnn7-gcc54-devel (not ready) | [Dockerfile.cuda10.1-cudnn7-gcc54.devel](../tools/Dockerfile.cuda10.1-cudnn7-gcc54.devel) |
|              GPU (cuda10.1-cudnn7-tensorRT6) development               | Ubuntu16 | 0.8.0-cuda10.1-cudnn7-devel | [Dockerfile.cuda10.1-cudnn7.devel](../tools/Dockerfile.cuda10.1-cudnn7.devel) |
|              GPU (cuda10.2-cudnn7-tensorRT6) development               | Ubuntu16 | 0.8.0-cuda10.2-cudnn7-devel | [Dockerfile.cuda10.2-cudnn7.devel](../tools/Dockerfile.cuda10.2-cudnn7.devel) |
|              GPU (cuda10.2-cudnn8-tensorRT7) development               | Ubuntu16 | 0.8.0-cuda10.2-cudnn8-devel | [Dockerfile.cuda10.2-cudnn8.devel](../tools/Dockerfile.cuda10.2-cudnn8.devel) |
|              GPU (cuda11.2-cudnn8-tensorRT8) development               | Ubuntu16 | 0.8.0-cuda11.2-cudnn8-devel | [Dockerfile.cuda11.2-cudnn8.devel](../tools/Dockerfile.cuda11.2-cudnn8.devel) |

**Java Client:**
```
registry.baidubce.com/paddlepaddle/serving:0.8.0-cuda10.2-java
```

**XPU:**
```
registry.baidubce.com/paddlepaddle/serving:xpu-arm # for arm xpu user
registry.baidubce.com/paddlepaddle/serving:xpu-x86 # for x86 xpu user
```

## Requirements for running CUDA containers

Running a CUDA container requires a machine with at least one CUDA-capable GPU and a driver compatible with the CUDA toolkit version you are using. 

The machine running the CUDA container **only requires the NVIDIA driver**, the CUDA toolkit does not have to be installed.

For the relationship between CUDA toolkit version, Driver version and GPU architecture, please refer to [nvidia-docker wiki](https://github.com/NVIDIA/nvidia-docker/wiki/CUDA).

# (Attachment) The List of All the Docker images

Develop Images:

| Env      | Version | Docker images tag            | OS        | Gcc Version |
|----------|---------|------------------------------|-----------|-------------|
|    CPU   | >=0.5.0 | 0.8.0-devel                 | Ubuntu 16 |  8.2.0       |
|          | <=0.4.0 | 0.4.0-devel                  | CentOS 7  | 4.8.5       |
| Cuda10.1 | >=0.5.0 | 0.8.0-cuda10.1-cudnn7-devel  | Ubuntu 16 |   8.2.0       |
|          | <=0.4.0 | 0.4.0-cuda10.1-cudnn7-devel    | CentOS 7  | 4.8.5     |
| Cuda10.2+Cudnn7 | >=0.5.0 | 0.8.0-cuda10.2-cudnn7-devel  | Ubuntu 16 |   8.2.0       |
|          | <=0.4.0 | Nan                          | Nan       | Nan         |
| Cuda10.2+Cudnn8 | >=0.5.0 | 0.8.0-cuda10.2-cudnn8-devel  | Ubuntu 16 |   8.2.0       |
|          | <=0.4.0 | Nan                          | Nan       | Nan         |
| Cuda11.2 | >=0.5.0 | 0.8.0-cuda11.2-cudnn8-devel | Ubuntu 16 |    8.2.0       |
|          | <=0.4.0 | Nan                          | Nan       | Nan         |


Running Images:

Running Images is lighter than Develop Images, and Running Images are made up with serving whl and bin, but without develop tools like cmake because of lower image size. If you want to know about it, plese check the document [Paddle Serving on Kubernetes](./Run_On_Kubernetes_CN.md).


