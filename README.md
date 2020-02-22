# Paddle Serving
An easy-to-use Machine Learning Model Inference Service Deployment Tool

[![Release](https://img.shields.io/badge/Release-0.0.3-yellowgreen)](Release)
[![Issues](https://img.shields.io/github/issues/PaddlePaddle/Serving)](Issues)
[![License](https://img.shields.io/github/license/PaddlePaddle/Serving)](LICENSE)

[中文](./doc/README_CN.md)

Paddle Serving helps deep learning developers deploy an online inference service without much effort. Currently, Paddle Serving supports the deep learning models produced by [Paddle](https://github.com/PaddlePaddle/Paddle) althought it can be very easy to support other deep learning framework's model inference engine.  

## Key Features
- Integrate with Paddle training pipeline seemlessly, most paddle models can be deployed with one line command.
- Industrial serving features supported, such as multiple models management, model online loading, online A/B testing etc.
- Distributed Key-Value indexing supported that is especially useful for large scale sparse features as model inputs.
- Highly concurrent and efficient communication, with [Baidu-rpc](https://github.com/apache/incubator-brpc) supported.
- Multiple programming language supported on client side, such as Golang, C++ and python

## Quick Start

Paddle Serving supports light-weighted Python API for model inference and can be integrated with trainining process seemlessly. Here is a Boston House Pricing example for users to do quick start.

### Installation

```shell
pip install paddle-serving-client
pip install paddle-serving-server
```

### Download models and start server
``` shell
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/uci_housing.tar.gz
tar -xzf uci_housing.tar.gz
python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9292
```

### Client Side Scripts

```
from paddle_serving_client import Client
import paddle
import sys

client = Client()
client.load_client_config(sys.argv[1])
client.connect(["127.0.0.1:9292"])

test_reader = paddle.batch(paddle.reader.shuffle(
    paddle.dataset.uci_housing.test(), buf_size=500), batch_size=1)

for data in test_reader():
    fetch_map = client.predict(feed={"x": data[0][0]}, fetch=["price"])
    print("{} {}".format(fetch_map["price"][0], data[0][1][0]))

```



### Document

[Design Doc(Chinese)](doc/DESIGN.md)

[How to config Serving native operators on server side?](doc/SERVER_OP.md)

[How to develop a new Serving operator](doc/OPERATOR.md)

[Client API for other programming languages](doc/CLIENT_API.md)

[FAQ(Chinese)](doc/FAQ.md)

### Advanced features and development

[Compile from source code(Chinese)](doc/COMPILE.md)

## Contribution

If you want to contribute code to Paddle Serving, please reference [Contribution Guidelines](doc/CONTRIBUTE.md)
