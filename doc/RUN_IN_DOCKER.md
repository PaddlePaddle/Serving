# How to run PaddleServing in Docker

([简体中文](RUN_IN_DOCKER_CN.md)|English)

## Requirements

Docker (GPU version requires nvidia-docker to be installed on the GPU machine)

This document takes Python2 as an example to show how to run Paddle Serving in docker. You can also use Python3 to run related commands by replacing `python` with `python3`.

## CPU

### Get docker image

Refer to [this document](DOCKER_IMAGES.md) for a docker image:

```shell
docker pull hub.baidubce.com/paddlepaddle/serving:latest
```


### Create container

```bash
docker run -p 9292:9292 --name test -dit hub.baidubce.com/paddlepaddle/serving:latest
docker exec -it test bash
```

The `-p` option is to map the `9292` port of the container to the `9292` port of the host.

### Install PaddleServing

In order to make the image smaller, the PaddleServing package is not installed in the image. You can run the following command to install it:

```bash
pip install paddle-serving-server
```

You may need to use a domestic mirror source (in China, you can use the Tsinghua mirror source of the following example) to speed up the download:

```shell
pip install paddle-serving-server -i https://pypi.tuna.tsinghua.edu.cn/simple
```

### Test example

Get the trained Boston house price prediction model by the following command:

```bash
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/uci_housing.tar.gz
tar -xzf uci_housing.tar.gz
```

- Test HTTP service

  Running on the Server side (inside the container):

  ```bash
  python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9292 --name uci >std.log 2>err.log &
  ```

  Running on the Client side (inside or outside the container):

  ```bash
  curl -H "Content-Type:application/json" -X POST -d '{"feed":{"x": [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]}, "fetch":["price"]}' http://127.0.0.1:9292/uci/prediction
  ```

- Test RPC service

  Running on the Server side (inside the container):

  ```bash
  python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9292 >std.log 2>err.log &
  ```

  Running following Python code on the Client side (inside or outside the container, The `paddle-serving-client` package needs to be installed):

  ```bash
  from paddle_serving_client import Client
  
  client = Client()
  client.load_client_config("uci_housing_client/serving_client_conf.prototxt")
  client.connect(["127.0.0.1:9292"])
  data = [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727,
          -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]
  fetch_map = client.predict(feed={"x": data}, fetch=["price"])
  print(fetch_map)
  ```

  

## GPU

The GPU version is basically the same as the CPU version, with only some differences in interface naming (GPU version requires nvidia-docker to be installed on the GPU machine).

### Get docker image

Refer to [this document](DOCKER_IMAGES.md) for a docker image, the following is an example of an `cuda9.0-cudnn7` image:

```shell
nvidia-docker pull hub.baidubce.com/paddlepaddle/serving:latest-cuda9.0-cudnn7
```

### Create container

```bash
nvidia-docker run -p 9292:9292 --name test -dit hub.baidubce.com/paddlepaddle/serving:latest-cuda9.0-cudnn7
nvidia-docker exec -it test bash
```

The `-p` option is to map the `9292` port of the container to the `9292` port of the host.

### Install PaddleServing

In order to make the image smaller, the PaddleServing package is not installed in the image. You can run the following command to install it:

```bash
pip install paddle-serving-server-gpu
```

You may need to use a domestic mirror source (in China, you can use the Tsinghua mirror source of the following example) to speed up the download:

```shell
pip install paddle-serving-server-gpu -i https://pypi.tuna.tsinghua.edu.cn/simple
```

### Test example

When running the GPU Server, you need to set the GPUs used by the prediction service through the `--gpu_ids` option, and the CPU is used by default. An error will be reported when the value of `--gpu_ids` exceeds the environment variable `CUDA_VISIBLE_DEVICES`. The following example specifies to use a GPU with index 0:
```shell
export CUDA_VISIBLE_DEVICES=0,1
python -m paddle_serving_server_gpu.serve --model uci_housing_model --port 9292 --gpu_ids 0
```


Get the trained Boston house price prediction model by the following command:

```bash
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/uci_housing.tar.gz
tar -xzf uci_housing.tar.gz
```

- Test HTTP service

  Running on the Server side (inside the container):

  ```bash
  python -m paddle_serving_server_gpu.serve --model uci_housing_model --thread 10 --port 9292 --name uci --gpu_ids 0
  ```

  Running on the Client side (inside or outside the container):

  ```bash
  curl -H "Content-Type:application/json" -X POST -d '{"feed":{"x": [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]}, "fetch":["price"]}' http://127.0.0.1:9292/uci/prediction
  ```

- Test RPC service

  Running on the Server side (inside the container):

  ```bash
  python -m paddle_serving_server_gpu.serve --model uci_housing_model --thread 10 --port 9292 --gpu_ids 0
  ```

  Running following Python code on the Client side (inside or outside the container, The `paddle-serving-client` package needs to be installed):

  ```bash
  from paddle_serving_client import Client
  
  client = Client()
  client.load_client_config("uci_housing_client/serving_client_conf.prototxt")
  client.connect(["127.0.0.1:9292"])
  data = [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727,
          -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]
  fetch_map = client.predict(feed={"x": data}, fetch=["price"])
  print(fetch_map)
  ```




## Attention

Runtime images cannot be used for compilation. If you want to compile from source, refer to [COMPILE](COMPILE.md).
