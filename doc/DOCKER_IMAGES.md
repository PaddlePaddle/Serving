# Docker Images

([简体中文](DOCKER_IMAGES_CN.md)|English)

This document maintains a list of docker images provided by Paddle Serving.

## Get docker image

You can get images in two ways:

1. Pull image directly from `hub.baidubce.com ` or `docker.io` through TAG:

   ```shell
   docker pull hub.baidubce.com/paddlepaddle/serving:<TAG> # hub.baidubce.com
   docker pull paddlepaddle/serving:<TAG> # hub.docker.com
   ```

2. Building image based on dockerfile

   Create a new folder and copy Dockerfile to this folder, and run the following command:

   ```shell
   docker build -t <image-name>:<images-tag> .
   ```



## Image description

Runtime images cannot be used for compilation.
If you want to customize your Serving based on source code, use the version with the suffix - devel.

|                         Description                          |   OS    |             TAG              |                          Dockerfile                          |
| :----------------------------------------------------------: | :-----: | :--------------------------: | :----------------------------------------------------------: |
|                         CPU runtime                          | CentOS7 |            latest            |              [Dockerfile](../tools/Dockerfile)               |
|                       CPU development                        | CentOS7 |         latest-devel         |        [Dockerfile.devel](../tools/Dockerfile.devel)         |
|                 GPU (cuda9.0-cudnn7) runtime                 | CentOS7 |    latest-cuda9.0-cudnn7     | [Dockerfile.cuda9.0-cudnn7](../tools/Dockerfile.cuda9.0-cudnn7) |
|               GPU (cuda9.0-cudnn7) development               | CentOS7 | latest-cuda9.0-cudnn7-devel  | [Dockerfile.cuda9.0-cudnn7.devel](../tools/Dockerfile.cuda9.0-cudnn7.devel) |
|                GPU (cuda10.0-cudnn7) runtime                 | CentOS7 |    latest-cuda10.0-cudnn7    | [Dockerfile.cuda10.0-cudnn7](../tools/Dockerfile.cuda10.0-cudnn7) |
|              GPU (cuda10.0-cudnn7) development               | CentOS7 | latest-cuda10.0-cudnn7-devel | [Dockerfile.cuda10.0-cudnn7.devel](../tools/Dockerfile.cuda10.0-cudnn7.devel) |
|     CPU development (Used to compile packages on Ubuntu)     | CentOS6 |            <None>            | [Dockerfile.centos6.devel](../tools/Dockerfile.centos6.devel) |
| GPU (cuda9.0-cudnn7) development (Used to compile packages on Ubuntu) | CentOS6 |            <None>            | [Dockerfile.centos6.cuda9.0-cudnn7.devel](../tools/Dockerfile.centos6.cuda9.0-cudnn7.devel) |



## Requirements for running CUDA containers

Running a CUDA container requires a machine with at least one CUDA-capable GPU and a driver compatible with the CUDA toolkit version you are using. 

The machine running the CUDA container **only requires the NVIDIA driver**, the CUDA toolkit doesn't have to be installed.

For the relationship between CUDA toolkit version, Driver version and GPU architecture, please refer to [nvidia-docker wiki](https://github.com/NVIDIA/nvidia-docker/wiki/CUDA).
