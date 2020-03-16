<img src='https://paddle-serving.bj.bcebos.com/imdb-demo%2FLogoMakr-3Bd2NM-300dpi.png' width = "600" height = "127">

[![Build Status](https://img.shields.io/travis/com/PaddlePaddle/Serving/develop)](https://travis-ci.com/PaddlePaddle/Serving)
[![Release](https://img.shields.io/badge/Release-0.0.3-yellowgreen)](Release)
[![Issues](https://img.shields.io/github/issues/PaddlePaddle/Serving)](Issues)
[![License](https://img.shields.io/github/license/PaddlePaddle/Serving)](LICENSE)
[![Slack](https://img.shields.io/badge/Join-Slack-green)](https://paddleserving.slack.com/archives/CU0PB4K35)

## 动机
Paddle Serving 帮助深度学习开发者轻易部署在线预测服务。 **本项目目标**: 只要你使用 [Paddle](https://github.com/PaddlePaddle/Paddle) 训练了一个深度神经网络，你就同时拥有了该模型的预测服务。
<p align="center">
    <img src="doc/demo.gif" width="700">
</p>

## 核心功能
- 与Paddle训练紧密连接，绝大部分Paddle模型可以 **一键部署**.
- 支持 **工业级的服务能力** 例如模型管理，在线加载，在线A/B测试等.
- 支持 **分布式键值对索引** 助力于大规模稀疏特征作为模型输入.
- 支持客户端和服务端之间 **高并发和高效通信**.
- 支持 **多种编程语言** 开发客户端，例如Golang，C++和Python.
- **可伸缩框架设计** 可支持不限于Paddle的模型服务.

## 安装

```shell
pip install paddle-serving-client
pip install paddle-serving-server
```

## 快速启动示例

``` shell
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/uci_housing.tar.gz
tar -xzf uci_housing.tar.gz
python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9292
```

Python客户端请求

``` python
from paddle_serving_client import Client

client = Client()
client.load_client_config("uci_housing_client/serving_client_conf.prototxt")
client.connect(["127.0.0.1:9292"])
data = [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727,
        -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]
fetch_map = client.predict(feed={"x": data}, fetch=["price"])
print(fetch_map)

```

## 文档

[开发文档](doc/DESIGN.md)

[如何在服务器端配置本地Op?](doc/SERVER_DAG.md)

[如何开发一个新的Op?](doc/NEW_OPERATOR.md)

[Golang 客户端](doc/IMDB_GO_CLIENT.md)

[从源码编译](doc/COMPILE.md)

[常见问答](doc/FAQ.md)

## 加入社区
如果您想要联系其他用户和开发者，欢迎加入我们的 [Slack channel](https://paddleserving.slack.com/archives/CUBPKHKMJ)

## 如何贡献代码

如果您想要贡献代码给Paddle Serving，请参考[Contribution Guidelines](doc/CONTRIBUTE.md)
