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

| 镜像说明                                           | 操作系统 | TAG                          | Dockerfile                                                   |
| -------------------------------------------------- | -------- | ---------------------------- | ------------------------------------------------------------ |
| CPU 运行镜像                                       | CentOS7  | latest                       | [Dockerfile](../tools/Dockerfile)                            |
| CPU 开发镜像                                       | CentOS7  | latest-devel                 | [Dockerfile.devel](../tools/Dockerfile.devel)                |
| GPU (cuda9.0-cudnn7) 运行镜像                      | CentOS7  | latest-cuda9.0-cudnn7        | [Dockerfile.cuda9.0-cudnn7](../tools/Dockerfile.cuda9.0-cudnn7) |
| GPU (cuda9.0-cudnn7) 开发镜像                      | CentOS7  | latest-cuda9.0-cudnn7-devel  | [Dockerfile.cuda9.0-cudnn7.devel](../tools/Dockerfile.cuda9.0-cudnn7.devel) |
| GPU (cuda10.0-cudnn7) 运行镜像                     | CentOS7  | latest-cuda10.0-cudnn7       | [Dockerfile.cuda10.0-cudnn7](../tools/Dockerfile.cuda10.0-cudnn7) |
| GPU (cuda10.0-cudnn7) 开发镜像                     | CentOS7  | latest-cuda10.0-cudnn7-devel | [Dockerfile.cuda10.0-cudnn7.devel](../tools/Dockerfile.cuda10.0-cudnn7.devel) |
| CPU 开发镜像 (用于编译 Ubuntu 包)                  | CentOS6  | <无>                         | [Dockerfile.centos6.devel](../tools/Dockerfile.centos6.devel) |
| GPU (cuda9.0-cudnn7) 开发镜像 (用于编译 Ubuntu 包) | CentOS6  | <无>                         | [Dockerfile.centos6.cuda9.0-cudnn7.devel](../tools/Dockerfile.centos6.cuda9.0-cudnn7.devel) |
