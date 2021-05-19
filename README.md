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

<h3 align="center">Some Key Features of Paddle Serving</h3>

- Integrate with Paddle training pipeline seamlessly, most paddle models can be deployed **with one line command**.
- **Industrial serving features** supported, such as models management, online loading, online A/B testing etc.
- **Highly concurrent and efficient communication** between clients and servers supported.
- **Multiple programming languages** supported on client side, such as C++, python and Java.

***

- Any model trained by [PaddlePaddle](https://github.com/paddlepaddle/paddle) can be directly used or [Model Conversion Interface](./doc/SAVE.md) for online deployment of Paddle Serving.
- Support [Multi-model Pipeline Deployment](./doc/PIPELINE_SERVING.md), and provide the requirements of the REST interface and RPC interface itself, [Pipeline example](./python/examples/pipeline).
- Support the model zoos from the Paddle ecosystem, such as [PaddleDetection](./python/examples/detection), [PaddleOCR](./python/examples/ocr), [PaddleRec](https://github.com/PaddlePaddle/PaddleRec/tree/master/recserving/movie_recommender).
- Provide a variety of pre-processing and post-processing to facilitate users in training, deployment and other stages of related code, bridging the gap between AI developers and application developers, please refer to
[Serving Examples](./python/examples/).

<p align="center">
    <img src="doc/demo.gif" width="700">
</p>


<h2 align="center">AIStudio Turorial</h2>

Here we provide tutorial on AIStudio(Chinese Version) [AIStudio教程-Paddle Serving服务化部署框架](https://www.paddlepaddle.org.cn/tutorials/projectdetail/1555945)

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

**Attention:**: Currently, the default GPU environment of paddlepaddle 2.1 is Cuda 10.2, so the sample code of GPU Docker is based on Cuda 10.2. We also provides docker images and whl packages for other GPU environments. If users use other environments, they need to carefully check and select the appropriate version.

**Attention:** the following so-called 'python' or 'pip' stands for one of Python 3.6/3.7/3.8.

```
# Run CPU Docker
docker pull registry.baidubce.com/paddlepaddle/serving:0.6.0-devel
docker run -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/serving:0.6.0-devel bash
docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving
```
```
# Run GPU Docker
nvidia-docker pull registry.baidubce.com/paddlepaddle/serving:0.6.0-cuda10.2-cudnn8-devel
nvidia-docker run -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/serving:0.6.0-cuda10.2-cudnn8-devel bash
nvidia-docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving
```
install python dependencies
```
cd Serving
pip install -r python/requirements.txt
```

```shell
pip install paddle-serving-client==0.6.0
pip install paddle-serving-server==0.6.0 # CPU
pip install paddle-serving-app==0.6.0
pip install paddle-serving-server-gpu==0.6.0.post102 #GPU with CUDA10.2 + TensorRT7
# DO NOT RUN ALL COMMANDS! check your GPU env and select the right one
pip install paddle-serving-server-gpu==0.6.0.post101 # GPU with CUDA10.1 + TensorRT6
pip install paddle-serving-server-gpu==0.6.0.post11 # GPU with CUDA10.1 + TensorRT7
```

You may need to use a domestic mirror source (in China, you can use the Tsinghua mirror source, add `-i https://pypi.tuna.tsinghua.edu.cn/simple` to pip command) to speed up the download.

If you need install modules compiled with develop branch, please download packages from [latest packages list](./doc/LATEST_PACKAGES.md) and install with `pip install` command. If you want to compile by yourself, please refer to [How to compile Paddle Serving?](./doc/COMPILE.md)

Packages of paddle-serving-server and paddle-serving-server-gpu support Centos 6/7, Ubuntu 16/18, Windows 10.

Packages of paddle-serving-client and paddle-serving-app support Linux and Windows, but paddle-serving-client only support python3.6/3.7/3.8.

**For latest version, Cuda 9.0 or Cuda 10.0 are no longer supported, Python2.7/3.5 is no longer supported.**

Recommended to install paddle >= 2.1.0


```
# CPU users, please run
pip install paddlepaddle==2.1.0

# GPU Cuda10.2 please run
pip install paddlepaddle-gpu==2.1.0 
```

**Note**: If your Cuda version is not 10.2, please do not execute the above commands directly, you need to refer to [Paddle official documentation-multi-version whl package list
](https://www.paddlepaddle.org.cn/documentation/docs/en/install/Tables_en.html#multi-version-whl-package-list-release)

Select the url link of the corresponding GPU environment and install it. For example, for Python3.6 users of Cuda 10.1, please select `cp36-cp36m` and
The url corresponding to `cuda10.1-cudnn7-mkl-gcc8.2-avx-trt6.0.1.5`, copy it and run
```
pip install https://paddle-wheel.bj.bcebos.com/with-trt/2.1.0-gpu-cuda10.1-cudnn7-mkl-gcc8.2/paddlepaddle_gpu-2.1.0.post101-cp36-cp36m-linux_x86_64.whl
```

the default `paddlepaddle-gpu==2.1.0` is Cuda 10.2 with no TensorRT. If you want to install PaddlePaddle with TensorRT. please also check the documentation-multi-version whl package list and find key word `cuda10.2-cudnn8.0-trt7.1.3`. More info please check [Paddle Serving uses TensorRT](./doc/TENSOR_RT.md)

If it is other environment and Python version, please find the corresponding link in the table and install it with pip.


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

| Argument                                       | Type | Default | Description                                           |
| ---------------------------------------------- | ---- | ------- | ----------------------------------------------------- |
| `thread`                                       | int  | `4`     | Concurrency of current service                        |
| `port`                                         | int  | `9292`  | Exposed port of current service to users              |
| `model`                                        | str  | `""`    | Path of paddle model directory to be served           |
| `mem_optim_off`                                | -    | -       | Disable memory / graphic memory optimization          |
| `ir_optim`                                     | bool | False   | Enable analysis and optimization of calculation graph |
| `use_mkl` (Only for cpu version)               | -    | -       | Run inference with MKL                                |
| `use_trt` (Only for trt version)               | -    | -       | Run inference with TensorRT                           |
| `use_lite` (Only for Intel x86 CPU or ARM CPU) | -    | -       | Run PaddleLite inference                              |
| `use_xpu`                                      | -    | -       | Run PaddleLite inference with Baidu Kunlun XPU        |
| `precision`                                    | str  | FP32    | Precision Mode, support FP32, FP16, INT8              |
| `use_calib`                                    | bool | False   | Only for deployment with TensorRT                     |

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
<h3 align="center">Pipeline Service</h3>

Paddle Serving provides industry-leading multi-model tandem services, which strongly supports the actual operating business scenarios of major companies, please refer to [OCR word recognition](./python/examples/pipeline/ocr).

we get two models
```
python -m paddle_serving_app.package --get_model ocr_rec
tar -xzvf ocr_rec.tar.gz
python -m paddle_serving_app.package --get_model ocr_det
tar -xzvf ocr_det.tar.gz
```
then we start server side, launch two models as one standalone web service
```
python web_service.py
```
http request
```
python pipeline_http_client.py
```
grpc request
```
python pipeline_rpc_client.py
```
output
```
{'err_no': 0, 'err_msg': '', 'key': ['res'], 'value': ["['土地整治与土壤修复研究中心', '华南农业大学1素图']"]}
```


<h2 align="center">Document</h2>

### New to Paddle Serving
- [How to save a servable model?](doc/SAVE.md)
- [Write Bert-as-Service in 10 minutes](doc/BERT_10_MINS.md)
- [Paddle Serving Examples](python/examples)
- [How to process natural data in Paddle Serving?(Chinese)](doc/PROCESS_DATA.md)
- [How to process level of detail(LOD)?](doc/LOD.md)

### Developers
- [How to deploy Paddle Serving on K8S?(Chinese)](doc/PADDLE_SERVING_ON_KUBERNETES.md)
- [How to route Paddle Serving to secure endpoint?(Chinese)](doc/SERVIING_AUTH_DOCKER.md)
- [How to develop a new Web Service?](doc/NEW_WEB_SERVICE.md)
- [Compile from source code](doc/COMPILE.md)
- [Develop Pipeline Serving](doc/PIPELINE_SERVING.md)
- [Deploy Web Service with uWSGI](doc/UWSGI_DEPLOY.md)
- [Hot loading for model file](doc/HOT_LOADING_IN_SERVING.md)
- [Paddle Serving uses TensorRT](doc/TENSOR_RT.md)

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
- Special Thanks to [@cuicheng01](https://github.com/cuicheng01) for providing 11 PaddleClas models

### Feedback

For any feedback or to report a bug, please propose a [GitHub Issue](https://github.com/PaddlePaddle/Serving/issues).

### License

[Apache 2.0 License](https://github.com/PaddlePaddle/Serving/blob/develop/LICENSE)
