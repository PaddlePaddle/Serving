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
pip install paddle-serving-client==0.4.0 
pip install paddle-serving-server==0.4.0 # CPU
pip install paddle-serving-server-gpu==0.4.0.post9 # GPU with CUDA9.0
pip install paddle-serving-server-gpu==0.4.0.post10 # GPU with CUDA10.0
pip install paddle-serving-server-gpu==0.4.0.trt # GPU with CUDA10.1+TensorRT
```

You may need to use a domestic mirror source (in China, you can use the Tsinghua mirror source, add `-i https://pypi.tuna.tsinghua.edu.cn/simple` to pip command) to speed up the download.

If you need install modules compiled with develop branch, please download packages from [latest packages list](./doc/LATEST_PACKAGES.md) and install with `pip install` command.

Packages of paddle-serving-server and paddle-serving-server-gpu support Centos 6/7, Ubuntu 16/18, Windows 10.

Packages of paddle-serving-client and paddle-serving-app support Linux and Windows, but paddle-serving-client only support python2.7/3.5/3.6/3.7.

Recommended to install paddle >= 1.8.4.

For **Windows Users**, please read the document [Paddle Serving for Windows Users](./doc/WINDOWS_TUTORIAL.md)

<h2 align="center"> Pre-built services with Paddle Serving</h2>

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

### RPC service

A user can also start a RPC service with `paddle_serving_server.serve`. RPC service is usually faster than HTTP service, although a user needs to do some coding based on Paddle Serving's python client API. Note that we do not specify `--name` here. 
``` shell
python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9292
```
<center>

| Argument | Type | Default | Description |
|--------------|------|-----------|--------------------------------|
| `thread` | int | `4` | Concurrency of current service |
| `port` | int | `9292` | Exposed port of current service to users|
| `model` | str | `""` | Path of paddle model directory to be served |
| `mem_optim_off` | - | - | Disable memory / graphic memory optimization |
| `ir_optim` | - | - | Enable analysis and optimization of calculation graph |
| `use_mkl` (Only for cpu version) | - | - | Run inference with MKL |
| `use_trt` (Only for trt version) | - | - | Run inference with TensorRT  |

</center>

```python
# A user can visit rpc service through paddle_serving_client API
from paddle_serving_client import Client
import numpy as np
client = Client()
client.load_client_config("uci_housing_client/serving_client_conf.prototxt")
client.connect(["127.0.0.1:9292"])
data = [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727,
        -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]
fetch_map = client.predict(feed={"x": np.array(data).reshape(1,13,1)}, fetch=["price"])
print(fetch_map)
```
Here, `client.predict` function has two arguments. `feed` is a `python dict` with model input variable alias name and values. `fetch` assigns the prediction variables to be returned from servers. In the example, the name of `"x"` and `"price"` are assigned when the servable model is saved during training.


### WEB service

Users can also put the data format processing logic on the server side, so that they can directly use curl to access the service, refer to the following case whose path is `python/examples/fit_a_line`

```python
from paddle_serving_server.web_service import WebService
import numpy as np

class UciService(WebService):
    def preprocess(self, feed=[], fetch=[]):
        feed_batch = []
        is_batch = True
        new_data = np.zeros((len(feed), 1, 13)).astype("float32")
        for i, ins in enumerate(feed):
            nums = np.array(ins["x"]).reshape(1, 1, 13)
            new_data[i] = nums
        feed = {"x": new_data}
        return feed, fetch, is_batch

uci_service = UciService(name="uci")
uci_service.load_model_config("uci_housing_model")
uci_service.prepare_server(workdir="workdir", port=9292)
uci_service.run_rpc_service()
uci_service.run_web_service()
```
for client side,
```
curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"x": [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]}], "fetch":["price"]}' http://127.0.0.1:9292/uci/prediction
```
the response is
```
{"result":{"price":[[18.901151657104492]]}}
```

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

### Tutorial at AIStudio
- [Introduction to PaddleServing](https://aistudio.baidu.com/aistudio/projectdetail/605819)
- [Image Segmentation on Paddle Serving](https://aistudio.baidu.com/aistudio/projectdetail/457715)
- [Sentimental Analysis](https://aistudio.baidu.com/aistudio/projectdetail/509014)

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
