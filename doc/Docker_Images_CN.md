# Docker 镜像

(简体中文|[English](Docker_Images_EN.md))

该文档维护了 Paddle Serving 提供的镜像列表。

## 获取镜像

您可以通过两种方式获取镜像。

1. 通过 TAG 直接从 dockerhub 或 `registry.baidubce.com` 拉取镜像，具体TAG请参见下文的**镜像说明**章节的表格。

   ```shell
   docker pull registry.baidubce.com/paddlepaddle/serving:<TAG> 
   ```

2. 基于 Dockerfile 构建镜像

   建立新目录，复制对应 Dockerfile 内容到该目录下 Dockerfile 文件。执行

   ```shell
   docker build -f tools/${DOCKERFILE} -t <image-name>:<images-tag> .
   ```
   

## 镜像说明

若需要基于源代码二次开发编译，请使用后缀为-devel的版本。
**在TAG列，0.7.0也可以替换成对应的版本号，例如0.5.0/0.4.1等，但需要注意的是，部分开发环境随着某个版本迭代才增加，因此并非所有环境都有对应的版本号可以使用。**


|                         镜像选择                         |   操作系统    |             TAG              |                          Dockerfile                          |
| :----------------------------------------------------------: | :-----: | :--------------------------: | :----------------------------------------------------------: |
|                       CPU development                        | Ubuntu16 |         0.8.0-devel         |        [Dockerfile.devel](../tools/Dockerfile.devel)         |
|              GPU (cuda10.1-cudnn7-tensorRT6-gcc54) development               | Ubuntu16 | 0.8.0-cuda10.1-cudnn7-gcc54-devel (not ready) | [Dockerfile.cuda10.1-cudnn7-gcc54.devel](../tools/Dockerfile.cuda10.1-cudnn7-gcc54.devel) |
|              GPU (cuda10.1-cudnn7-tensorRT6) development               | Ubuntu16 | 0.8.0-cuda10.1-cudnn7-devel | [Dockerfile.cuda10.1-cudnn7.devel](../tools/Dockerfile.cuda10.1-cudnn7.devel) |
|              GPU (cuda10.2-cudnn7-tensorRT6) development               | Ubuntu16 | 0.8.0-cuda10.2-cudnn7-devel | [Dockerfile.cuda10.2-cudnn7.devel](../tools/Dockerfile.cuda10.2-cudnn7.devel) |
|              GPU (cuda10.2-cudnn8-tensorRT7) development               | Ubuntu16 | 0.8.0-cuda10.2-cudnn8-devel | [Dockerfile.cuda10.2-cudnn8.devel](../tools/Dockerfile.cuda10.2-cudnn8.devel) |
|              GPU (cuda11.2-cudnn8-tensorRT8) development               | Ubuntu16 | 0.8.0-cuda11.2-cudnn8-devel | [Dockerfile.cuda11.2-cudnn8.devel](../tools/Dockerfile.cuda11.2-cudnn8.devel) |

**Java镜像：**
```
registry.baidubce.com/paddlepaddle/serving:0.8.0-cuda10.2-java
```

**XPU镜像：**
```
registry.baidubce.com/paddlepaddle/serving:xpu-arm # for arm xpu user
registry.baidubce.com/paddlepaddle/serving:xpu-x86 # for x86 xpu user
```


## 运行CUDA容器的要求

运行CUDA容器需要至少具有一个支持CUDA的GPU以及与您所使用的CUDA工具包版本兼容的驱动程序。

运行CUDA容器的机器**只需要相应的NVIDIA驱动程序**，而CUDA工具包不是必要的。

相关CUDA工具包版本、驱动版本和GPU架构的关系请参阅 [nvidia-docker wiki](https://github.com/NVIDIA/nvidia-docker/wiki/CUDA)。

# （附录）所有镜像列表


开发镜像:

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

运行镜像:

运行镜像比开发镜像更加轻量化, 运行镜像提供了serving的whl和bin，但为了运行期更小的镜像体积，没有提供诸如cmake这样但开发工具。 如果您想了解有关信息，请检查文档[在Kubernetes上使用Paddle Serving](./Run_On_Kubernetes_CN.md)。


