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

- [Motivation](./README.md#motivation)
- [AIStudio Tutorial](./README.md#aistuio-tutorial)
- [Installation](./README.md#installation)
- [Quick Start Example](./README.md#quick-start-example)
- [Document](README.md#document)
- [Community](README.md#community)

<h2 align="center">Motivation</h2>

We consider deploying deep learning inference service online to be a user-facing application in the future. **The goal of this project**: When you have trained a deep neural net with [Paddle](https://github.com/PaddlePaddle/Paddle), you are also capable to deploy the model online easily. A demo of Paddle Serving is as follows:

- Any model trained by [PaddlePaddle](https://github.com/paddlepaddle/paddle) can be directly used or [Model Conversion Interface](./doc/SAVE_CN.md) for online deployment of Paddle Serving.
- Support [Multi-model Pipeline Deployment](./doc/PIPELINE_SERVING.md), and provide the requirements of the REST interface and RPC interface itself, [Pipeline example](./python/examples/pipeline).
- Support the major model libraries of the Paddle ecosystem, such as [PaddleDetection](./python/examples/detection), [PaddleOCR](./python/examples/ocr), [PaddleRec](https://github.com/PaddlePaddle/PaddleRec/tree/master/tools/recserving/movie_recommender).
- Provide a variety of pre-processing and post-processing to facilitate users in training, deployment and other stages of related code, bridging the gap between AI developers and application developers, please refer to
[Serving Examples](./python/examples/).

<p align="center">
    <img src="doc/demo.gif" width="700">
</p>


<h2 align="center">AIStudio Turorial</h2>

Here we provide tutorial on AIStudio(Chinese Version) [AIStudio教程-Paddle Serving服务化部署框架](https://aistudio.baidu.com/aistudio/projectdetail/1550674)

The tutorial provides 
<ul>
<li>Paddle Serving Environment Setup</li>
  <ul>
    <li>Running in docker images
    <li>pip install Paddle Serving
  </ul>
<li>Quick Experience of Paddle Serving</li>
<li>Advanced Tutorial of Model Deployment</li>
  <ul>
    <li>Save/Convert Models for Paddle Serving</li>
    <li>Setup Online Inference Service</li>
  </ul>
<li>Paddle Serving Examples</li>
  <ul>
    <li>Paddle Serving for Detections</li>
    <li>Paddle Serving for OCR</li>
  </ul>
</ul>


<h2 align="center">Installation</h2>

We **highly recommend** you to **run Paddle Serving in Docker**, please visit [Run in Docker](doc/RUN_IN_DOCKER.md). See the [document](doc/DOCKER_IMAGES.md) for more docker images.

**Attention:**: Currently, the default GPU environment of paddlepaddle 2.0 is Cuda 10.2, so the sample code of GPU Docker is based on Cuda 10.2. We also provides docker images and whl packages for other GPU environments. If users use other environments, they need to carefully check and select the appropriate version.

```
# Run CPU Docker
docker pull registry.baidubce.com/paddlepaddle/serving:0.5.0-devel
docker run -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/serving:0.5.0-devel
docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving
```
```
# Run GPU Docker
nvidia-docker pull registry.baidubce.com/paddlepaddle/serving:0.5.0-cuda10.2-cudnn8-devel
nvidia-docker run -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/serving:0.5.0-cuda10.2-cudnn8-devel
nvidia-docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving
```

```shell
pip install paddle-serving-client==0.5.0
pip install paddle-serving-server==0.5.0 # CPU
pip install paddle-serving-app==0.2.0
pip install paddle-serving-server-gpu==0.5.0.post102 #GPU with CUDA10.2 + TensorRT7
# DO NOT RUN ALL COMMANDS! check your GPU env and select the right one
pip install paddle-serving-server-gpu==0.5.0.post9 # GPU with CUDA9.0
pip install paddle-serving-server-gpu==0.5.0.post10 # GPU with CUDA10.0
pip install paddle-serving-server-gpu==0.5.0.post101 # GPU with CUDA10.1 + TensorRT6
pip install paddle-serving-server-gpu==0.5.0.post11 # GPU with CUDA10.1 + TensorRT7
```

You may need to use a domestic mirror source (in China, you can use the Tsinghua mirror source, add `-i https://pypi.tuna.tsinghua.edu.cn/simple` to pip command) to speed up the download.

If you need install modules compiled with develop branch, please download packages from [latest packages list](./doc/LATEST_PACKAGES.md) and install with `pip install` command. If you want to compile by yourself, please refer to [How to compile Paddle Serving?](./doc/COMPILE.md)

Packages of paddle-serving-server and paddle-serving-server-gpu support Centos 6/7, Ubuntu 16/18, Windows 10.

Packages of paddle-serving-client and paddle-serving-app support Linux and Windows, but paddle-serving-client only support python2.7/3.5/3.6/3.7/3.8.

Recommended to install paddle >= 2.0.0

```
# CPU users, please run
pip install paddlepaddle==2.0.0

# GPU Cuda10.2 please run
pip install paddlepaddle-gpu==2.0.0
```

For **Windows Users**, please read the document [Paddle Serving for Windows Users](./doc/WINDOWS_TUTORIAL.md)

<h2 align="center">Quick Start Example</h2>

This quick start example is mainly for those users who already have a model to deploy, and we also provide a model that can be used for deployment. in case if you want to know how to complete the process from offline training to online service, please refer to the AiStudio tutorial above.

### Boston House Price Prediction model

get into the Serving git directory, and change dir to `fit_a_line`
``` shell
cd Serving/python/examples/fit_a_line
sh get_data.sh
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
| `use_lite` (Only for ARM) | - | - | Run PaddleLite inference |
| `use_xpu` (Only for ARM+XPU) | - | - | Run PaddleLite XPU inference |

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

```
python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9292 --name uci
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
- [Write Bert-as-Service in 10 minutes](doc/BERT_10_MINS.md)
- [Paddle Serving Examples](python/examples)

### Developers
- [How to develop a new Web Service?](doc/NEW_WEB_SERVICE.md)
- [Compile from source code](doc/COMPILE.md)
- [Develop Pipeline Serving](doc/PIPELINE_SERVING.md)
- [Deploy Web Service with uWSGI](doc/UWSGI_DEPLOY.md)
- [Hot loading for model file](doc/HOT_LOADING_IN_SERVING.md)

### About Efficiency
- [How to profile Paddle Serving latency?](python/examples/util)
- [How to optimize performance?](doc/PERFORMANCE_OPTIM.md)
- [Deploy multi-services on one GPU(Chinese)](doc/MULTI_SERVICE_ON_ONE_GPU_CN.md)
- [CPU Benchmarks(Chinese)](doc/BENCHMARKING.md)
- [GPU Benchmarks(Chinese)](doc/GPU_BENCHMARKING.md)

### Design
- [Design Doc](doc/DESIGN_DOC.md)

### FAQ
- [FAQ(Chinese)](doc/FAQ.md)

<h2 align="center">Community</h2>

### Slack

To connect with other users and contributors, welcome to join our [Slack channel](https://paddleserving.slack.com/archives/CUBPKHKMJ)

### Contribution

If you want to contribute code to Paddle Serving, please reference [Contribution Guidelines](doc/CONTRIBUTE.md)

- Special Thanks to [@BeyondYourself](https://github.com/BeyondYourself) in complementing the gRPC tutorial, updating the FAQ doc and modifying the mdkir command
- Special Thanks to [@mcl-stone](https://github.com/mcl-stone) in updating faster_rcnn benchmark
- Special Thanks to [@cg82616424](https://github.com/cg82616424) in updating the unet benchmark and modifying resize comment error

### Feedback

For any feedback or to report a bug, please propose a [GitHub Issue](https://github.com/PaddlePaddle/Serving/issues).

### License

[Apache 2.0 License](https://github.com/PaddlePaddle/Serving/blob/develop/LICENSE)
