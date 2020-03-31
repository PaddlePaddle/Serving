# How to run PaddleServing in Docker

([简体中文](./RUN_IN_DOCKER_CN.md)|English)

## Requirements

Docker (GPU version requires nvidia-docker to be installed on the GPU machine)

## CPU

### Get docker image

You can get images in two ways:

1. Pull image directly

   ```bash
   docker pull hub.baidubce.com/paddlepaddle/serving:0.1.3
   ```

2. Building image based on dockerfile

   Create a new folder and copy [Dockerfile](../tools/Dockerfile) to this folder, and run the following command:

   ```bash
   docker build -t hub.baidubce.com/paddlepaddle/serving:0.1.3 .
   ```

### Create container

```bash
docker run -p 9292:9292 --name test -dit hub.baidubce.com/paddlepaddle/serving:0.1.3
docker exec -it test bash
```

The `-p` option is to map the `9292` port of the container to the `9292` port of the host.

### Install PaddleServing

In order to make the image smaller, the PaddleServing package is not installed in the image. You can run the following command to install it

```bash
pip install paddle-serving-server
```

### Test example

Before running the GPU version of the Server side code, you need to set the `CUDA_VISIBLE_DEVICES` environment variable to specify which GPUs the prediction service uses. The following example specifies two GPUs with indexes 0 and 1:

```bash
export CUDA_VISIBLE_DEVICES=0,1
```

Get the trained Boston house price prediction model by the following command:

```bash
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/uci_housing.tar.gz
tar -xzf uci_housing.tar.gz
```

- Test HTTP service

  Running on the Server side (inside the container):

  ```bash
  python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9292 --name uci &>std.log 2>err.log &
  ```

  Running on the Client side (inside or outside the container):

  ```bash
  curl -H "Content-Type:application/json" -X POST -d '{"x": [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332], "fetch":["price"]}' http://127.0.0.1:9292/uci/prediction
  ```

- Test RPC service

  Running on the Server side (inside the container):

  ```bash
  python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9292 &>std.log 2>err.log &
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

You can also get images in two ways:

1. Pull image directly

   ```bash
   nvidia-docker pull hub.baidubce.com/paddlepaddle/serving:0.1.3-gpu
   ```

2. Building image based on dockerfile

   Create a new folder and copy [Dockerfile.gpu](../tools/Dockerfile.gpu) to this folder, and run the following command:

   ```bash
   nvidia-docker build -t hub.baidubce.com/paddlepaddle/serving:0.1.3-gpu .
   ```

### Create container

```bash
nvidia-docker run -p 9292:9292 --name test -dit hub.baidubce.com/paddlepaddle/serving:0.1.3-gpu
nvidia-docker exec -it test bash
```

The `-p` option is to map the `9292` port of the container to the `9292` port of the host.

### Install PaddleServing

In order to make the image smaller, the PaddleServing package is not installed in the image. You can run the following command to install it:

```bash
pip install paddle-serving-server-gpu
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
  python -m paddle_serving_server_gpu.serve --model uci_housing_model --thread 10 --port 9292 --name uci --gpu_ids 0
  ```

  Running on the Client side (inside or outside the container):

  ```bash
  curl -H "Content-Type:application/json" -X POST -d '{"x": [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332], "fetch":["price"]}' http://127.0.0.1:9292/uci/prediction
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

  
