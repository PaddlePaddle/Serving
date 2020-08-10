([简体中文](./README_CN.md)|English)

<p align="center">
    <br>
<img src='doc/serving_logo.png' width = "600" height = "130">
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


<h2 align="center">Installation</h2>

We **highly recommend** you to **run Paddle Serving in Docker**, please visit [Run in Docker](https://github.com/PaddlePaddle/Serving/blob/develop/doc/RUN_IN_DOCKER.md). See the [document](doc/DOCKER_IMAGES.md) for more docker images.
```
# Run CPU Docker
docker pull hub.baidubce.com/paddlepaddle/serving:latest
docker run -p 9292:9292 --name test -dit hub.baidubce.com/paddlepaddle/serving:latest
docker exec -it test bash
```
```
# Run GPU Docker
nvidia-docker pull hub.baidubce.com/paddlepaddle/serving:latest-cuda9.0-cudnn7
nvidia-docker run -p 9292:9292 --name test -dit hub.baidubce.com/paddlepaddle/serving:latest-cuda9.0-cudnn7
nvidia-docker exec -it test bash
```

```shell
pip install paddle-serving-client 
pip install paddle-serving-server # CPU
pip install paddle-serving-server-gpu # GPU
```

You may need to use a domestic mirror source (in China, you can use the Tsinghua mirror source, add `-i https://pypi.tuna.tsinghua.edu.cn/simple` to pip command) to speed up the download.

If you need install modules compiled with develop branch, please download packages from [latest packages list](./doc/LATEST_PACKAGES.md) and install with `pip install` command.

Packages of paddle-serving-server and paddle-serving-server-gpu support Centos 6/7 and Ubuntu 16/18.

Packages of paddle-serving-client and paddle-serving-app support Linux and Windows, but paddle-serving-client only support python2.7/3.6/3.7.

Recommended to install paddle >= 1.8.2.

<h2 align="center"> Pre-built services with Paddle Serving</h2>

<h3 align="center">Latest release</h4>
<p align="center">
    <a href="https://github.com/PaddlePaddle/Serving/tree/develop/python/examples/ocr">Optical Character Recognition</a>
    <br>
    <a href="https://github.com/PaddlePaddle/Serving/tree/develop/python/examples/faster_rcnn_model">Object Detection</a>
    <br>
    <a href="https://github.com/PaddlePaddle/Serving/tree/develop/python/examples/deeplabv3">Image Segmentation</a>
<p>

<h3 align="center">Chinese Word Segmentation</h4>

``` shell
> python -m paddle_serving_app.package --get_model lac
> tar -xzf lac.tar.gz
> python lac_web_service.py lac_model/ lac_workdir 9393 &
> curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"words": "我爱北京天安门"}], "fetch":["word_seg"]}' http://127.0.0.1:9393/lac/prediction
{"result":[{"word_seg":"我|爱|北京|天安门"}]}
```

<h3 align="center">Image Classification</h4>

<p align="center">
    <br>
<img src='https://paddle-serving.bj.bcebos.com/imagenet-example/daisy.jpg' width = "200" height = "200">
    <br>
<p>

``` shell
> python -m paddle_serving_app.package --get_model resnet_v2_50_imagenet
> tar -xzf resnet_v2_50_imagenet.tar.gz
> python resnet50_imagenet_classify.py resnet50_serving_model &
> curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"image": "https://paddle-serving.bj.bcebos.com/imagenet-example/daisy.jpg"}], "fetch": ["score"]}' http://127.0.0.1:9292/image/prediction
{"result":{"label":["daisy"],"prob":[0.9341403245925903]}}
```


<h2 align="center">Quick Start Example</h2>

This quick start example is only for users who already have a model to deploy and we prepare a ready-to-deploy model here. If you want to know how to use paddle serving from offline training to online serving, please reference to [Train_To_Service](https://github.com/PaddlePaddle/Serving/blob/develop/doc/TRAIN_TO_SERVICE.md)

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
| `mem_optim_off` | - | - | Disable memory / graphic memory optimization |
| `ir_optim` | - | - | Enable analysis and optimization of calculation graph |
| `use_mkl` (Only for cpu version) | - | - | Run inference with MKL |

Here, we use `curl` to send a HTTP POST request to the service we just started. Users can use any python library to send HTTP POST as well, e.g, [requests](https://requests.readthedocs.io/en/master/).
</center>

``` shell
curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"x": [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]}], "fetch":["price"]}' http://127.0.0.1:9292/uci/prediction
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

<h2 align="center">Some Key Features of Paddle Serving</h2>

- Integrate with Paddle training pipeline seamlessly, most paddle models can be deployed **with one line command**.
- **Industrial serving features** supported, such as models management, online loading, online A/B testing etc.
- **Distributed Key-Value indexing** supported which is especially useful for large scale sparse features as model inputs.
- **Highly concurrent and efficient communication** between clients and servers supported.
- **Multiple programming languages** supported on client side, such as Golang, C++ and python.

<h2 align="center">Document</h2>

### New to Paddle Serving
- [How to save a servable model?](doc/SAVE.md)
- [An End-to-end tutorial from training to inference service deployment](doc/TRAIN_TO_SERVICE.md)
- [Write Bert-as-Service in 10 minutes](doc/BERT_10_MINS.md)

### Developers
- [How to config Serving native operators on server side?](doc/SERVER_DAG.md)
- [How to develop a new Serving operator?](doc/NEW_OPERATOR.md)
- [How to develop a new Web Service?](doc/NEW_WEB_SERVICE.md)
- [Golang client](doc/IMDB_GO_CLIENT.md)
- [Compile from source code](doc/COMPILE.md)
- [Deploy Web Service with uWSGI](doc/UWSGI_DEPLOY.md)
- [Hot loading for model file](doc/HOT_LOADING_IN_SERVING.md)

### About Efficiency
- [How to profile Paddle Serving latency?](python/examples/util)
- [How to optimize performance?](doc/PERFORMANCE_OPTIM.md)
- [Deploy multi-services on one GPU(Chinese)](doc/MULTI_SERVICE_ON_ONE_GPU_CN.md)
- [CPU Benchmarks(Chinese)](doc/BENCHMARKING.md)
- [GPU Benchmarks(Chinese)](doc/GPU_BENCHMARKING.md)

### FAQ
- [FAQ(Chinese)](doc/FAQ.md)


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
