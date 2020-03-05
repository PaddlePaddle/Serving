<img src='https://paddle-serving.bj.bcebos.com/imdb-demo%2FLogoMakr-3Bd2NM-300dpi.png' width = "600" height = "127">

[![Release](https://img.shields.io/badge/Release-0.0.3-yellowgreen)](Release)
[![Issues](https://img.shields.io/github/issues/PaddlePaddle/Serving)](Issues)
[![License](https://img.shields.io/github/license/PaddlePaddle/Serving)](LICENSE)
[![Slack](https://img.shields.io/badge/Join-Slack-green)](https://paddleserving.slack.com/archives/CU0PB4K35)

[中文](https://github.com/PaddlePaddle/Serving/blob/develop/README_CN.md)

## Motivation
Paddle Serving helps deep learning developers deploy an online inference service without much effort. **The goal of this project**: once you have trained a deep neural nets with [Paddle](https://github.com/PaddlePaddle/Paddle), you already have a model inference service. A demo of serving is as follows:
<p align="center">
    <img src="doc/demo.gif" width="700">
</p>

## Key Features
- Integrate with Paddle training pipeline seemlessly, most paddle models can be deployed **with one line command**.
- **Industrial serving features** supported, such as models management, online loading, online A/B testing etc.
- **Distributed Key-Value indexing** supported that is especially useful for large scale sparse features as model inputs.
- **Highly concurrent and efficient communication** between clients and servers.
- **Multiple programming languages** supported on client side, such as Golang, C++ and python
- **Extensible framework design** that can support model serving beyond Paddle.

## Installation

```shell
pip install paddle-serving-client
pip install paddle-serving-server
```

## Quick Start Example

### Boston House Price Prediction model
``` shell
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/uci_housing.tar.gz
tar -xzf uci_housing.tar.gz
```

Paddle Serving provides HTTP and RPC based service for users to access

### HTTP service

``` shell
python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9292 --name uci
```
``` shell
curl -H "Content-Type:application/json" -X POST -d '{"x": [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332], "fetch":["price"]}' http://127.0.0.1:9292/uci/prediction
```

### RPC service

``` shell
python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9292
```

``` python
# A user can visit rpc service through paddle_serving_client API
from paddle_serving_client import Client

client = Client()
client.load_client_config("uci_housing_client/serving_client_conf.prototxt")
client.connect(["127.0.0.1:9292"])
data = [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727,
        -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]
fetch_map = client.predict(feed={"x": data}, fetch=["price"])
print(fetch_map)

```

## Models waiting for you to deploy


<center>

|      Model Name      	|              Resnet50              	|
|:--------------------:	|:----------------------------------:	|
|      Package URL     	|           To be released           	|
|      Description     	| Get the representation of an image 	|
| Training Data Source 	|              Imagenet              	|

</center>


## Document

[How to save a servable model?](doc/SAVE.md)

[How to config Serving native operators on server side?](doc/SERVER_DAG.md)

[How to develop a new Serving operator](doc/NEW_OPERATOR.md)

[Golang client](doc/IMDB_GO_CLIENT.md)

[Compile from source code(Chinese)](doc/COMPILE.md)

[FAQ(Chinese)](doc/FAQ.md)

[Design Doc(Chinese)](doc/DESIGN.md)

## Join Community
To connect with other users and contributors, welcome to join our [Slack channel](https://paddleserving.slack.com/archives/CUBPKHKMJ)

## Contribution

If you want to contribute code to Paddle Serving, please reference [Contribution Guidelines](doc/CONTRIBUTE.md)

### Feedback
For any feedback or to report a bug, please propose a [GitHub Issue](https://github.com/PaddlePaddle/Serving/issues).

## License
[Apache 2.0 License](https://github.com/PaddlePaddle/Serving/blob/develop/LICENSE)
