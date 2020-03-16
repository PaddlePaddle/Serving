# 如何在Docker中运行PaddleServing

## 环境要求

Docker（GPU版本需要在GPU机器上安装nvidia-docker）

## CPU版本

### 获取镜像

可以通过两种方式获取镜像。

1. 直接拉取镜像

   ```bash
   docker pull hub.baidubce.com/ctr/paddleserving:0.1.3
   ```

2. 基于Dockerfile构建镜像

   建立新目录，复制[Dockerfile](../tools/Dockerfile)内容到该目录下Dockerfile文件。执行

   ```bash
   docker build -t hub.baidubce.com/ctr/paddleserving:0.1.3 .
   ```

### 创建容器并进入

```bash
docker run -p 9292:9292 --name test -dit hub.baidubce.com/ctr/paddleserving:0.1.3
docker exec -it test bash
```

`-p`选项是为了将容器的`9292`端口映射到宿主机的`9292`端口。

### 安装PaddleServing

为了减小镜像的体积，镜像中没有安装Serving包，要执行下面命令进行安装

```bash
pip install paddle-serving-server
```

### 测试example

通过下面命令获取训练好的Boston房价预估模型：

```bash
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/uci_housing.tar.gz
tar -xzf uci_housing.tar.gz
```

- 测试HTTP服务

  在Server端（容器内）运行：

  ```bash
  python -m paddle_serving_server.web_serve --model uci_housing_model --thread 10 --port 9292 --name uci &>std.log 2>err.log &
  ```

  在Client端（容器内或容器外）运行：

  ```bash
  curl -H "Content-Type:application/json" -X POST -d '{"x": [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332], "fetch":["price"]}' http://127.0.0.1:9292/uci/prediction
  ```

- 测试RPC服务

  在Server端（容器内）运行：

  ```bash
  python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9292 &>std.log 2>err.log &
  ```

  在Client端（容器内或容器外，需要安装`paddle-serving-client`包）运行下面Python代码：

  ```python
  from paddle_serving_client import Client
  
  client = Client()
  client.load_client_config("uci_housing_client/serving_client_conf.prototxt")
  client.connect(["127.0.0.1:9292"])
  data = [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727,
          -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]
  fetch_map = client.predict(feed={"x": data}, fetch=["price"])
  print(fetch_map)
  ```

## GPU版本

GPU版本与CPU版本基本一致，只有部分接口命名的差别（GPU版本需要在GPU机器上安装nvidia-docker）。

### 获取镜像

可以通过两种方式获取镜像。

1. 直接拉取镜像

   ```bash
   nvidia-docker pull hub.baidubce.com/ctr/paddleserving:0.1.3-gpu
   ```

2. 基于Dockerfile构建镜像

   建立新目录，复制[Dockerfile.gpu](../tools/Dockerfile.gpu)内容到该目录下Dockerfile文件。执行

   ```bash
   nvidia-docker build -t hub.baidubce.com/ctr/paddleserving:0.1.3-gpu .
   ```

### 创建容器并进入

```bash
nvidia-docker run -p 9292:9292 --name test -dit hub.baidubce.com/ctr/paddleserving:0.1.3-gpu
nvidia-docker exec -it test bash
```

`-p`选项是为了将容器的`9292`端口映射到宿主机的`9292`端口。

### 安装PaddleServing

为了减小镜像的体积，镜像中没有安装Serving包，要执行下面命令进行安装

```bash
pip install paddle-serving-server-gpu
```

### 测试example

通过下面命令获取训练好的Boston房价预估模型：

```bash
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/uci_housing.tar.gz
tar -xzf uci_housing.tar.gz
```

- 测试HTTP服务

  在Server端（容器内）运行：

  ```bash
  python -m paddle_serving_server_gpu.web_serve --model uci_housing_model --thread 10 --port 9292 --name uci 
  ```

  在Client端（容器内或容器外）运行：

  ```bash
  curl -H "Content-Type:application/json" -X POST -d '{"x": [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332], "fetch":["price"]}' http://127.0.0.1:9292/uci/prediction
  ```

- 测试RPC服务

  在Server端（容器内）运行：

  ```bash
  python -m paddle_serving_server_gpu.serve --model uci_housing_model --thread 10 --port 9292
  ```

  在Client端（容器内或容器外，需要安装`paddle-serving-client`包）运行下面Python代码：

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

