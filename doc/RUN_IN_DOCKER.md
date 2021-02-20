# How to run PaddleServing in Docker

([简体中文](RUN_IN_DOCKER_CN.md)|English)

One of the biggest benefits of Docker is portability, which can be deployed on multiple operating systems and mainstream cloud computing platforms. The Paddle Serving Docker image can be deployed on Linux, Mac and Windows platforms.

## Requirements

Docker (GPU version requires nvidia-docker to be installed on the GPU machine)

This document takes Python2 as an example to show how to run Paddle Serving in docker. You can also use Python3 to run related commands by replacing `python` with `python3`.

## CPU

### Get docker image

Refer to [this document](DOCKER_IMAGES.md) for a docker image:

```shell
docker pull registry.baidubce.com/paddlepaddle/serving:latest-devel
```


### Create container

```bash
docker run -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/serving:latest-devel
docker exec -it test bash
```

The `-p` option is to map the `9292` port of the container to the `9292` port of the host.

### Install PaddleServing

The mirror comes with `paddle_serving_server`, `paddle_serving_client`, and `paddle_serving_app` corresponding to the mirror tag version. If users don’t need to change the version, they can use it directly, which is suitable for environments without extranet services.

If you need to change the version, please refer to the instructions on the homepage to download the pip package of the corresponding version.
  

## GPU

The GPU version is basically the same as the CPU version, with only some differences in interface naming (GPU version requires nvidia-docker to be installed on the GPU machine).

### Get docker image

Refer to [this document](DOCKER_IMAGES.md) for a docker image, the following is an example of an `cuda9.0-cudnn7` image:

```shell
docker pull registry.baidubce.com/paddlepaddle/serving:latest-cuda10.2-cudnn8-devel
```

### Create container

```bash
nvidia-docker run -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/serving:latest-cuda10.2-cudnn8-devel
nvidia-docker exec -it test bash
```

or

```bash
docker run --gpus all -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/serving:latest-cuda10.2-cudnn8-devel
docker exec -it test bash
```

The `-p` option is to map the `9292` port of the container to the `9292` port of the host.

### Install PaddleServing

The mirror comes with `paddle_serving_server_gpu`, `paddle_serving_client`, and `paddle_serving_app` corresponding to the mirror tag version. If users don’t need to change the version, they can use it directly, which is suitable for environments without extranet services.

If you need to change the version, please refer to the instructions on the homepage to download the pip package of the corresponding version. [LATEST_PACKAGES](./LATEST_PACKAGES.md)

## Precautious

Runtime images cannot be used for compilation. If you want to compile from source, refer to [COMPILE](COMPILE.md).
