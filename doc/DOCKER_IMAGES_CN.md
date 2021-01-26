# Docker 镜像

(简体中文|[English](DOCKER_IMAGES.md))

该文档维护了 Paddle Serving 提供的镜像列表。

## 获取镜像

您可以通过两种方式获取镜像。

1. 通过 TAG 直接从 `hub.baidubce.com ` 或 `docker.io` 拉取镜像：

   ```shell
   docker pull hub.baidubce.com/paddlepaddle/serving:<TAG> # hub.baidubce.com
   docker pull paddlepaddle/serving:<TAG> # hub.docker.com
   ```

2. 基于 Dockerfile 构建镜像

   建立新目录，复制对应 Dockerfile 内容到该目录下 Dockerfile 文件。执行

   ```shell
   docker build -t <image-name>:<images-tag> .
   ```
   


## 镜像说明

运行时镜像不能用于开发编译。
若需要基于源代码二次开发编译，请使用后缀为-devel的版本。

|                         镜像选择                         |   操作系统    |             TAG              |                          Dockerfile                          |
| :----------------------------------------------------------: | :-----: | :--------------------------: | :----------------------------------------------------------: |
|                         CPU 运行镜像                          | CentOS7 |            latest            |              [Dockerfile](../tools/Dockerfile)               |
|                       CPU 开发镜像                        | CentOS7 |         latest-devel         |        [Dockerfile.devel](../tools/Dockerfile.devel)         |
|                 GPU (cuda9.0-cudnn7) 运行镜像                 | CentOS7 |    latest-cuda9.0-cudnn7     | [Dockerfile.cuda9.0-cudnn7](../tools/Dockerfile.cuda9.0-cudnn7) |
|               GPU (cuda9.0-cudnn7) 开发镜像          | CentOS7 | latest-cuda9.0-cudnn7-devel  | [Dockerfile.cuda9.0-cudnn7.devel](../tools/Dockerfile.cuda9.0-cudnn7.devel) |
|                GPU (cuda10.0-cudnn7) 运行镜像                 | CentOS7 |    latest-cuda10.0-cudnn7    | [Dockerfile.cuda10.0-cudnn7](../tools/Dockerfile.cuda10.0-cudnn7) |
|              GPU (cuda10.0-cudnn7) 开发镜像               | CentOS7 | latest-cuda10.0-cudnn7-devel | [Dockerfile.cuda10.0-cudnn7.devel](../tools/Dockerfile.cuda10.0-cudnn7.devel) |
|                GPU (cuda10.1-cudnn7-tensorRT6) 运行镜像                 | Ubuntu16 |    latest-cuda10.1-cudnn7    | [Dockerfile.cuda10.1-cudnn7](../tools/Dockerfile.cuda10.1-cudnn7) |
|              GPU (cuda10.1-cudnn7-tensorRT6) 开发镜像               | Ubuntu16 | latest-cuda10.1-cudnn7-devel | [Dockerfile.cuda10.1-cudnn7.devel](../tools/Dockerfile.cuda10.1-cudnn7.devel) |
|                GPU (cuda10.2-cudnn8-tensorRT7) 运行镜像                 | Ubuntu16|    latest-cuda10.2-cudnn8   | [Dockerfile.cuda10.2-cudnn8](../tools/Dockerfile.cuda10.2-cudnn8) |
|              GPU (cuda10.2-cudnn8-tensorRT7) 开发镜像               | Ubuntu16 | latest-cuda10.2-cudnn8-devel | [Dockerfile.cuda10.2-cudnn8.devel](../tools/Dockerfile.cuda10.2-cudnn8.devel) |
|                GPU (cuda11-cudnn8-tensorRT7) 运行镜像                 | Ubuntu18|    latest-cuda11-cudnn8   | [Dockerfile.cuda11-cudnn8](../tools/Dockerfile.cuda11-cudnn8) |
|              GPU (cuda11-cudnn8-tensorRT7) 开发镜像               | Ubuntu18 | latest-cuda11-cudnn8-devel | [Dockerfile.cuda11-cudnn8.devel](../tools/Dockerfile.cuda11-cudnn8.devel) |


## 运行CUDA容器的要求

运行CUDA容器需要至少具有一个支持CUDA的GPU以及与您所使用的CUDA工具包版本兼容的驱动程序。

运行CUDA容器的机器**只需要相应的NVIDIA驱动程序**，而CUDA工具包不是必要的。

相关CUDA工具包版本、驱动版本和GPU架构的关系请参阅 [nvidia-docker wiki](https://github.com/NVIDIA/nvidia-docker/wiki/CUDA)。
