# 如何在Docker中运行PaddleServing

(简体中文|[English](RUN_IN_DOCKER.md))

Docker最大的好处之一就是可移植性，可在多种操作系统和主流的云计算平台部署。使用Paddle Serving Docker镜像可在Linux、Mac和Windows平台部署。

## 环境要求

Docker（GPU版本需要在GPU机器上安装nvidia-docker）

该文档以Python2为例展示如何在Docker中运行Paddle Serving，您也可以通过将`python`更换成`python3`来用Python3运行相关命令。

## CPU版本

### 获取镜像

参考[该文档](DOCKER_IMAGES_CN.md)获取镜像：

以CPU编译镜像为例

```shell
docker pull registry.baidubce.com/paddlepaddle/serving:latest-devel
```

### 创建容器并进入

```bash
docker run -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/serving:latest-devel
docker exec -it test bash
```

`-p`选项是为了将容器的`9292`端口映射到宿主机的`9292`端口。

### 安装PaddleServing

镜像里自带对应镜像tag版本的`paddle_serving_server`，`paddle_serving_client`，`paddle_serving_app`，如果用户不需要更改版本，可以直接使用，适用于没有外网服务的环境。

如果需要更换版本，请参照首页的指导，下载对应版本的pip包。

## GPU 版本

```shell
docker pull registry.baidubce.com/paddlepaddle/serving:latest-cuda10.2-cudnn8-devel
```

### 创建容器并进入

```bash
nvidia-docker run -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/serving:latest-cuda10.2-cudnn8-devel
nvidia-docker exec -it test bash
```
或者
```bash
docker run --gpus all -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/serving:latest-cuda10.2-cudnn8-devel
docker exec -it test bash
```

`-p`选项是为了将容器的`9292`端口映射到宿主机的`9292`端口。

### 安装PaddleServing

镜像里自带对应镜像tag版本的`paddle_serving_server_gpu`，`paddle_serving_client`，`paddle_serving_app`，如果用户不需要更改版本，可以直接使用，适用于没有外网服务的环境。

如果需要更换版本，请参照首页的指导，下载对应版本的pip包。[最新安装包合集](LATEST_PACKAGES.md)

## 注意事项

运行时镜像不能用于开发编译。如果想要从源码编译，请查看[如何编译PaddleServing](COMPILE.md)。
