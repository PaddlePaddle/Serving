<p align="center">
    <br>
<img src='https://paddle-serving.bj.bcebos.com/imdb-demo%2FLogoMakr-3Bd2NM-300dpi.png' width = "600" height = "130">
    <br>
<p>
    
<p align="center">
    <br>
    <a href="https://travis-ci.com/PaddlePaddle/Serving">
        <img alt="Build Status" src="https://img.shields.io/travis/com/PaddlePaddle/Serving/develop">
    </a>
    <img alt="Release" src="https://img.shields.io/badge/Release-0.0.3-yellowgreen">
    <img alt="Issues" src="https://img.shields.io/github/issues/PaddlePaddle/Serving">
    <img alt="License" src="https://img.shields.io/github/license/PaddlePaddle/Serving">
    <img alt="Slack" src="https://img.shields.io/badge/Join-Slack-green">
    <br>
<p>

<h2 align="center">Motivation</h2>

We consider deploying deep learning inference service online to be a user-facing application in the future. **The goal of this project**: When you have trained a deep neural net with [Paddle](https://github.com/PaddlePaddle/Paddle), you are also capable to deploy the model online easily. A demo of Paddle Serving is as follows:
<p align="center">
    <img src="doc/demo.gif" width="700">
</p>

<h2 align="center">Some Key Features</h2>

- Integrate with Paddle training pipeline seamlessly, most paddle models can be deployed **with one line command**.
- **Industrial serving features** supported, such as models management, online loading, online A/B testing etc.
- **Distributed Key-Value indexing** supported which is especially useful for large scale sparse features as model inputs.
- **Highly concurrent and efficient communication** between clients and servers supported.
- **Multiple programming languages** supported on client side, such as Golang, C++ and python.
- **Extensible framework design** which can support model serving beyond Paddle.

<h2 align="center">Installation</h2>

We highly recommend you to run Paddle Serving in Docker, please visit [Run in Docker](https://github.com/PaddlePaddle/Serving/blob/develop/doc/RUN_IN_DOCKER.md)

```shell
pip install paddle-serving-client
pip install paddle-serving-server
```

<h2 align="center">Quick Start Example</h2>

### Boston House Price Prediction model
``` shell
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/uci_housing.tar.gz
tar -xzf uci_housing.tar.gz
```

Paddle Serving provides HTTP and RPC based service for users to access

### HTTP service

Paddle Serving provides a built-in python module called `paddle_serving_server.serve` that can start a RPC service or a http service with one-line command. If we specify the argument `--name uci`, it means that we will have a HTTP service with a url of `$IP:$PORT/uci/prediction`
``` shell
python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9292 --name uci
```
<center>

| Argument | Type | Default | Description |
|--------------|------|-----------|--------------------------------|
| `thread` | int | `4` | Concurrency of current service |
| `port` | int | `9292` | Exposed port of current service to users|
| `name` | str | `""` | Service name, can be used to generate HTTP request url |
| `model` | str | `""` | Path of paddle model directory to be served |
| `mem_optim` | bool | `False` | Enable memory optimization |

Here, we use `curl` to send a HTTP POST request to the service we just started. Users can use any python library to send HTTP POST as well, e.g, [requests](https://requests.readthedocs.io/en/master/).
</center>

``` shell
curl -H "Content-Type:application/json" -X POST -d '{"x": [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332], "fetch":["price"]}' http://127.0.0.1:9292/uci/prediction
```

### RPC service

A user can also start a RPC service with `paddle_serving_server.serve`. RPC service is usually faster than HTTP service, although a user needs to do some coding based on Paddle Serving's python client API. Note that we do not specify `--name` here. 
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
Here, `client.predict` function has two arguments. `feed` is a `python dict` with model input variable alias name and values. `fetch` assigns the prediction variables to be returned from servers. In the example, the name of `"x"` and `"price"` are assigned when the servable model is saved during training.

<h2 align="center"> Pre-built services with Paddle Serving</h2>

<h3 align="center">Chinese Word Segmentation</h4>

- **Description**: 
``` shell
Chinese word segmentation HTTP service that can be deployed with one line command.
```

- **Download Servable Package**: 
``` shell
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/lac/lac_model_jieba_web.tar.gz
```
- **Host web service**: 
``` shell
tar -xzf lac_model_jieba_web.tar.gz
python lac_web_service.py jieba_server_model/ lac_workdir 9292
```
- **Request sample**: 
``` shell
curl -H "Content-Type:application/json" -X POST -d '{"words": "我爱北京天安门", "fetch":["word_seg"]}' http://127.0.0.1:9292/lac/prediction
```
- **Request result**: 
``` shell
{"word_seg":"我|爱|北京|天安门"}
```

<h3 align="center">Image Classification</h4>

- **Description**: 
``` shell
Image classification trained with Imagenet dataset. A label and corresponding probability will be returned.
```

- **Download Servable Package**: 
``` shell
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/imagenet-example/imagenet_demo.tar.gz
```
- **Host web service**: 
``` shell
tar -xzf imagenet_demo.tar.gz
python image_classification_service_demo.py resnet50_serving_model
```
- **Request sample**: 

<p align="center">
    <br>
<img src='https://paddle-serving.bj.bcebos.com/imagenet-example/daisy.jpg' width = "200" height = "200">
    <br>
<p>
    
``` shell
curl -H "Content-Type:application/json" -X POST -d '{"url": "https://paddle-serving.bj.bcebos.com/imagenet-example/daisy.jpg", "fetch": ["score"]}' http://127.0.0.1:9292/image/prediction
```
- **Request result**: 
``` shell
{"label":"daisy","prob":0.9341403245925903}
```

<h3 align="center">More Demos</h3>

| Key                | Value                                                        |
| :----------------- | :----------------------------------------------------------- |
| Model Name         | Bert-Base-Baike                                              |
| URL                | [https://paddle-serving.bj.bcebos.com/bert_example/bert_seq128.tar.gz](https://paddle-serving.bj.bcebos.com/bert_example%2Fbert_seq128.tar.gz) |
| Client/Server Code | https://github.com/PaddlePaddle/Serving/tree/develop/python/examples/bert |
| Description        | Get semantic representation from a Chinese Sentence          |



| Key                | Value                                                        |
| :----------------- | :----------------------------------------------------------- |
| Model Name         | Resnet50-Imagenet                                            |
| URL                | [https://paddle-serving.bj.bcebos.com/imagenet-example/ResNet50_vd.tar.gz](https://paddle-serving.bj.bcebos.com/imagenet-example%2FResNet50_vd.tar.gz) |
| Client/Server Code | https://github.com/PaddlePaddle/Serving/tree/develop/python/examples/imagenet |
| Description        | Get image semantic representation from an image              |



| Key                | Value                                                        |
| :----------------- | :----------------------------------------------------------- |
| Model Name         | Resnet101-Imagenet                                           |
| URL                | https://paddle-serving.bj.bcebos.com/imagenet-example/ResNet101_vd.tar.gz |
| Client/Server Code | https://github.com/PaddlePaddle/Serving/tree/develop/python/examples/imagenet |
| Description        | Get image semantic representation from an image              |



| Key                | Value                                                        |
| :----------------- | :----------------------------------------------------------- |
| Model Name         | CNN-IMDB                                                     |
| URL                | https://paddle-serving.bj.bcebos.com/imdb-demo/imdb_model.tar.gz |
| Client/Server Code | https://github.com/PaddlePaddle/Serving/tree/develop/python/examples/imdb |
| Description        | Get category probability from an English Sentence            |



| Key                | Value                                                        |
| :----------------- | :----------------------------------------------------------- |
| Model Name         | LSTM-IMDB                                                    |
| URL                | https://paddle-serving.bj.bcebos.com/imdb-demo/imdb_model.tar.gz |
| Client/Server Code | https://github.com/PaddlePaddle/Serving/tree/develop/python/examples/imdb |
| Description        | Get category probability from an English Sentence            |



| Key                | Value                                                        |
| :----------------- | :----------------------------------------------------------- |
| Model Name         | BOW-IMDB                                                     |
| URL                | https://paddle-serving.bj.bcebos.com/imdb-demo/imdb_model.tar.gz |
| Client/Server Code | https://github.com/PaddlePaddle/Serving/tree/develop/python/examples/imdb |
| Description        | Get category probability from an English Sentence            |



| Key                | Value                                                        |
| :----------------- | :----------------------------------------------------------- |
| Model Name         | Jieba-LAC                                                    |
| URL                | https://paddle-serving.bj.bcebos.com/lac/lac_model.tar.gz    |
| Client/Server Code | https://github.com/PaddlePaddle/Serving/tree/develop/python/examples/lac |
| Description        | Get word segmentation from a Chinese Sentence                |



| Key                | Value                                                        |
| :----------------- | :----------------------------------------------------------- |
| Model Name         | DNN-CTR                                                      |
| URL                | None(Get model by [local_train.py](./python/examples/criteo_ctr/local_train.py))                            |
| Client/Server Code | https://github.com/PaddlePaddle/Serving/tree/develop/python/examples/criteo_ctr |
| Description        | Get click probability from a feature vector of item          |



| Key                | Value                                                        |
| :----------------- | :----------------------------------------------------------- |
| Model Name         | DNN-CTR(with cube)                                           |
| URL                | None(Get model by [local_train.py](python/examples/criteo_ctr_with_cube/local_train.py))                            |
| Client/Server Code | https://github.com/PaddlePaddle/Serving/tree/develop/python/examples/criteo_ctr_with_cube |
| Description        | Get click probability from a feature vector of item          |

<h2 align="center">Document</h2>

### New to Paddle Serving
- [How to save a servable model?](doc/SAVE.md)
- [An End-to-end tutorial from training to inference service deployment](doc/TRAIN_TO_SERVICE.md)
- [Write Bert-as-Service in 10 minutes](doc/BERT_10_MINS.md)

### Developers
- [How to config Serving native operators on server side?](doc/SERVER_DAG.md)
- [How to develop a new Serving operator?](doc/NEW_OPERATOR.md)
- [Golang client](doc/IMDB_GO_CLIENT.md)
- [Compile from source code](doc/COMPILE.md)

### About Efficiency
- [How to profile Paddle Serving latency?](python/examples/util)
- [CPU Benchmarks(Chinese)](doc/BENCHMARKING.md)
- [GPU Benchmarks(Chinese)](doc/GPU_BENCHMARKING.md)

### FAQ
- [FAQ(Chinese)](doc/deprecated/FAQ.md)


### Design
- [Design Doc](doc/DESIGN_DOC.md)

<h2 align="center">Community</h2>

### Slack

To connect with other users and contributors, welcome to join our [Slack channel](https://paddleserving.slack.com/archives/CUBPKHKMJ)

### Contribution

If you want to contribute code to Paddle Serving, please reference [Contribution Guidelines](doc/CONTRIBUTE.md)

### Feedback

For any feedback or to report a bug, please propose a [GitHub Issue](https://github.com/PaddlePaddle/Serving/issues).

### License

[Apache 2.0 License](https://github.com/PaddlePaddle/Serving/blob/develop/LICENSE)
