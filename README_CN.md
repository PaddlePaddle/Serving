(简体中文|[English](./README.md))

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

<h2 align="center">动机</h2>

Paddle Serving 旨在帮助深度学习开发者轻易部署在线预测服务。 **本项目目标**: 当用户使用 [Paddle](https://github.com/PaddlePaddle/Paddle) 训练了一个深度神经网络，就同时拥有了该模型的预测服务。

<p align="center">
    <img src="doc/demo.gif" width="700">
</p>



<h2 align="center">安装</h2>

**强烈建议**您在**Docker内构建**Paddle Serving，请查看[如何在Docker中运行PaddleServing](doc/RUN_IN_DOCKER_CN.md)。更多镜像请查看[Docker镜像列表](doc/DOCKER_IMAGES_CN.md)。

```
# 启动 CPU Docker
docker pull hub.baidubce.com/paddlepaddle/serving:latest
docker run -p 9292:9292 --name test -dit hub.baidubce.com/paddlepaddle/serving:latest
docker exec -it test bash
```
```
# 启动 GPU Docker
nvidia-docker pull hub.baidubce.com/paddlepaddle/serving:latest-cuda9.0-cudnn7
nvidia-docker run -p 9292:9292 --name test -dit hub.baidubce.com/paddlepaddle/serving:latest-cuda9.0-cudnn7
nvidia-docker exec -it test bash
```
```shell
pip install paddle-serving-client
pip install paddle-serving-server # CPU
pip install paddle-serving-server-gpu # GPU
```

您可能需要使用国内镜像源（例如清华源, 在pip命令中添加`-i https://pypi.tuna.tsinghua.edu.cn/simple`）来加速下载。

如果需要使用develop分支编译的安装包，请从[最新安装包列表](./doc/LATEST_PACKAGES.md)中获取下载地址进行下载，使用`pip install`命令进行安装。

paddle-serving-server和paddle-serving-server-gpu安装包支持Centos 6/7和Ubuntu 16/18。

paddle-serving-client和paddle-serving-app安装包支持Linux和Windows，其中paddle-serving-client仅支持python2.7/3.5/3.6。

推荐安装1.8.2及以上版本的paddle

<h2 align="center"> Paddle Serving预装的服务 </h2>

<h3 align="center">中文分词</h4>

``` shell
> python -m paddle_serving_app.package --get_model lac
> tar -xzf lac.tar.gz
> python lac_web_service.py lac_model/ lac_workdir 9393 &
> curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"words": "我爱北京天安门"}], "fetch":["word_seg"]}' http://127.0.0.1:9393/lac/prediction
{"result":[{"word_seg":"我|爱|北京|天安门"}]}
```

<h3 align="center">图像分类</h4>

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


<h2 align="center">快速开始示例</h2>

这个快速开始示例主要是为了给那些已经有一个要部署的模型的用户准备的，而且我们也提供了一个可以用来部署的模型。如果您想知道如何从离线训练到在线服务走完全流程，请参考[从训练到部署](https://github.com/PaddlePaddle/Serving/blob/develop/doc/TRAIN_TO_SERVICE_CN.md)

<h3 align="center">波士顿房价预测</h3>

``` shell
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/uci_housing.tar.gz
tar -xzf uci_housing.tar.gz
```

Paddle Serving 为用户提供了基于 HTTP 和 RPC 的服务


<h3 align="center">HTTP服务</h3>

Paddle Serving提供了一个名为`paddle_serving_server.serve`的内置python模块，可以使用单行命令启动RPC服务或HTTP服务。如果我们指定参数`--name uci`，则意味着我们将拥有一个HTTP服务，其URL为$IP:$PORT/uci/prediction`。

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
| `mem_optim_off` | - | - | Disable memory optimization |
| `ir_optim` | - | - | Enable analysis and optimization of calculation graph |
| `use_mkl` (Only for cpu version) | - | - | Run inference with MKL |

我们使用 `curl` 命令来发送HTTP POST请求给刚刚启动的服务。用户也可以调用python库来发送HTTP POST请求，请参考英文文档 [requests](https://requests.readthedocs.io/en/master/)。
</center>

``` shell
curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"x": [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]}], "fetch":["price"]}' http://127.0.0.1:9292/uci/prediction
```

<h3 align="center">RPC服务</h3>

用户还可以使用`paddle_serving_server.serve`启动RPC服务。 尽管用户需要基于Paddle Serving的python客户端API进行一些开发，但是RPC服务通常比HTTP服务更快。需要指出的是这里我们没有指定`--name`。

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
在这里，`client.predict`函数具有两个参数。 `feed`是带有模型输入变量别名和值的`python dict`。 `fetch`被要从服务器返回的预测变量赋值。 在该示例中，在训练过程中保存可服务模型时，被赋值的tensor名为`"x"`和`"price"`。

<h2 align="center">Paddle Serving的核心功能</h2>

- 与Paddle训练紧密连接，绝大部分Paddle模型可以 **一键部署**.
- 支持 **工业级的服务能力** 例如模型管理，在线加载，在线A/B测试等.
- 支持 **分布式键值对索引** 助力于大规模稀疏特征作为模型输入.
- 支持客户端和服务端之间 **高并发和高效通信**.
- 支持 **多种编程语言** 开发客户端，例如Golang，C++和Python.

<h2 align="center">文档</h2>

### 新手教程
- [怎样保存用于Paddle Serving的模型？](doc/SAVE_CN.md)
- [端到端完成从训练到部署全流程](doc/TRAIN_TO_SERVICE_CN.md)
- [十分钟构建Bert-As-Service](doc/BERT_10_MINS_CN.md)

### 开发者教程
- [如何配置Server端的计算图?](doc/SERVER_DAG_CN.md)
- [如何开发一个新的General Op?](doc/NEW_OPERATOR_CN.md)
- [如何开发一个新的Web Service?](doc/NEW_WEB_SERVICE_CN.md)
- [如何在Paddle Serving使用Go Client?](doc/IMDB_GO_CLIENT_CN.md)
- [如何编译PaddleServing?](doc/COMPILE_CN.md)
- [如何使用uWSGI部署Web Service](doc/UWSGI_DEPLOY_CN.md)
- [如何实现模型文件热加载](doc/HOT_LOADING_IN_SERVING_CN.md)

### 关于Paddle Serving性能
- [如何测试Paddle Serving性能？](python/examples/util/)
- [如何优化性能?](doc/PERFORMANCE_OPTIM_CN.md)
- [在一张GPU上启动多个预测服务](doc/MULTI_SERVICE_ON_ONE_GPU_CN.md)
- [CPU版Benchmarks](doc/BENCHMARKING.md)
- [GPU版Benchmarks](doc/GPU_BENCHMARKING.md)

### FAQ
- [常见问答](doc/FAQ.md)

### 设计文档
- [Paddle Serving设计文档](doc/DESIGN_DOC_CN.md)

<h2 align="center">社区</h2>

### Slack

想要同开发者和其他用户沟通吗？欢迎加入我们的 [Slack channel](https://paddleserving.slack.com/archives/CUBPKHKMJ)

### 贡献代码

如果您想为Paddle Serving贡献代码，请参考 [Contribution Guidelines](doc/CONTRIBUTE.md)

### 反馈

如有任何反馈或是bug，请在 [GitHub Issue](https://github.com/PaddlePaddle/Serving/issues)提交

### License

[Apache 2.0 License](https://github.com/PaddlePaddle/Serving/blob/develop/LICENSE)
