# 如何在Docker中运行PaddleServing

(简体中文|[English](RUN_IN_DOCKER.md))

## 环境要求

Docker（GPU版本需要在GPU机器上安装nvidia-docker）

该文档以Python2为例展示如何在Docker中运行Paddle Serving，您也可以通过将`python`更换成`python3`来用Python3运行相关命令。

## CPU版本

### 获取镜像

参考[该文档](DOCKER_IMAGES_CN.md)获取镜像：

```shell
docker pull hub.baidubce.com/paddlepaddle/serving:latest
```


### 创建容器并进入

```bash
docker run -p 9292:9292 --name test -dit hub.baidubce.com/paddlepaddle/serving:latest
docker exec -it test bash
```

`-p`选项是为了将容器的`9292`端口映射到宿主机的`9292`端口。

### 安装PaddleServing

为了减小镜像的体积，镜像中没有安装Serving包，要执行下面命令进行安装。

```bash
pip install paddle-serving-server
```

您可能需要使用国内镜像源（例如清华源）来加速下载。

```shell
pip install paddle-serving-server -i https://pypi.tuna.tsinghua.edu.cn/simple
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
  python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9292 --name uci >std.log 2>err.log &
  ```

  在Client端（容器内或容器外）运行：

  ```bash
  curl -H "Content-Type:application/json" -X POST -d '{"feed":{"x": [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]}, "fetch":["price"]}' http://127.0.0.1:9292/uci/prediction
  ```

- 测试RPC服务

  在Server端（容器内）运行：

  ```bash
  python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9292 >std.log 2>err.log &
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

参考[该文档](DOCKER_IMAGES_CN.md)获取镜像，这里以 `cuda9.0-cudnn7` 的镜像为例：

```shell
nvidia-docker pull hub.baidubce.com/paddlepaddle/serving:latest-cuda9.0-cudnn7
```

### 创建容器并进入

```bash
nvidia-docker run -p 9292:9292 --name test -dit hub.baidubce.com/paddlepaddle/serving:latest-cuda9.0-cudnn7
nvidia-docker exec -it test bash
```

`-p`选项是为了将容器的`9292`端口映射到宿主机的`9292`端口。

### 安装PaddleServing

为了减小镜像的体积，镜像中没有安装Serving包，要执行下面命令进行安装。

```bash
pip install paddle-serving-server-gpu
```

您可能需要使用国内镜像源（例如清华源）来加速下载。

```shell
pip install paddle-serving-server-gpu -i https://pypi.tuna.tsinghua.edu.cn/simple
```

### 测试example

在运行GPU版Server时需要通过`--gpu_ids`选项设置预测服务使用的GPU，缺省状态默认使用CPU。当设置的`--gpu_ids`超出环境变量`CUDA_VISIBLE_DEVICES`时会报错。下面的示例为指定使用索引为0的GPU：
```shell
export CUDA_VISIBLE_DEVICES=0,1
python -m paddle_serving_server_gpu.serve --model uci_housing_model --port 9292 --gpu_ids 0
```


通过下面命令获取训练好的Boston房价预估模型：

```bash
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/uci_housing.tar.gz
tar -xzf uci_housing.tar.gz
```

- 测试HTTP服务

  在Server端（容器内）运行：

  ```bash
  python -m paddle_serving_server_gpu.serve --model uci_housing_model --thread 10 --port 9292 --name uci --gpu_ids 0
  ```

  在Client端（容器内或容器外）运行：

  ```bash
  curl -H "Content-Type:application/json" -X POST -d '{"feed":{"x": [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]}, "fetch":["price"]}' http://127.0.0.1:9292/uci/prediction
  ```

- 测试RPC服务

  在Server端（容器内）运行：

  ```bash
  python -m paddle_serving_server_gpu.serve --model uci_housing_model --thread 10 --port 9292 --gpu_ids 0
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

## 注意事项

运行时镜像不能用于开发编译。如果想要从源码编译，请查看[如何编译PaddleServing](COMPILE.md)。
