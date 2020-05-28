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

<h2 align="center">核心功能</h2>

- 与Paddle训练紧密连接，绝大部分Paddle模型可以 **一键部署**.
- 支持 **工业级的服务能力** 例如模型管理，在线加载，在线A/B测试等.
- 支持 **分布式键值对索引** 助力于大规模稀疏特征作为模型输入.
- 支持客户端和服务端之间 **高并发和高效通信**.
- 支持 **多种编程语言** 开发客户端，例如Golang，C++和Python.
- **可伸缩框架设计** 可支持不限于Paddle的模型服务.

<h2 align="center">安装</h2>

**强烈建议**您在**Docker内构建**Paddle Serving，请查看[如何在Docker中运行PaddleServing](doc/RUN_IN_DOCKER_CN.md)

```
# 启动 CPU Docker
docker pull hub.baidubce.com/paddlepaddle/serving:latest
docker run -p 9292:9292 --name test -dit hub.baidubce.com/paddlepaddle/serving:latest
docker exec -it test bash
```
```
# 启动 GPU Docker
nvidia-docker pull hub.baidubce.com/paddlepaddle/serving:latest-gpu
nvidia-docker run -p 9292:9292 --name test -dit hub.baidubce.com/paddlepaddle/serving:latest-gpu
nvidia-docker exec -it test bash
```
```shell
pip install paddle-serving-client
pip install paddle-serving-server # CPU
pip install paddle-serving-server-gpu # GPU
```

您可能需要使用国内镜像源（例如清华源, 在pip命令中添加`-i https://pypi.tuna.tsinghua.edu.cn/simple`）来加速下载。

客户端安装包支持Centos 7和Ubuntu 18，或者您可以使用HTTP服务，这种情况下不需要安装客户端。

<h2 align="center">快速启动示例</h2>

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
| `mem_optim` | bool | `False` | Enable memory optimization |
| `ir_optim` | bool | `False` | Enable analysis and optimization of calculation graph |
| `use_mkl` (Only for cpu version) | bool | `False` | Run inference with MKL |

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

<h2 align="center">Paddle Serving预装的服务</h2>

<h3 align="center">中文分词模型</h4>

- **介绍**: 
``` shell
本示例为中文分词HTTP服务一键部署
```

- **下载服务包**: 
``` shell
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/lac/lac_model_jieba_web.tar.gz
```
- **启动web服务**: 
``` shell
tar -xzf lac_model_jieba_web.tar.gz
python lac_web_service.py jieba_server_model/ lac_workdir 9292
```
- **客户端请求示例**: 
``` shell
curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"words": "我爱北京天安门"}], "fetch":["word_seg"]}' http://127.0.0.1:9292/lac/prediction
```
- **返回结果示例**: 
``` shell
{"word_seg":"我|爱|北京|天安门"}
```

<h3 align="center">图像分类模型</h4>

- **介绍**: 
``` shell
图像分类模型由Imagenet数据集训练而成，该服务会返回一个标签及其概率
注意：本示例需要安装paddle-serving-server-gpu
```

- **下载服务包**: 
``` shell
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/imagenet-example/imagenet_demo.tar.gz
```
- **启动web服务**: 
``` shell
tar -xzf imagenet_demo.tar.gz
python image_classification_service_demo.py resnet50_serving_model
```
- **客户端请求示例**: 

<p align="center">
    <br>
<img src='https://paddle-serving.bj.bcebos.com/imagenet-example/daisy.jpg' width = "200" height = "200">
    <br>
<p>

``` shell
curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"url": "https://paddle-serving.bj.bcebos.com/imagenet-example/daisy.jpg"}], "fetch": ["score"]}' http://127.0.0.1:9292/image/prediction
```
- **返回结果示例**: 
``` shell
{"label":"daisy","prob":0.9341403245925903}
```

<h3 align="center">更多示例</h3>

| Key                | Value                                                        |
| :----------------- | :----------------------------------------------------------- |
| 模型名              | Bert-Base-Baike                                              |
| 下载链接                | [https://paddle-serving.bj.bcebos.com/bert_example/bert_seq128.tar.gz](https://paddle-serving.bj.bcebos.com/bert_example%2Fbert_seq128.tar.gz) |
| 客户端/服务端代码     | https://github.com/PaddlePaddle/Serving/tree/develop/python/examples/bert |
| 介绍                | 获得一个中文语句的语义表示          |



| Key                | Value                                                        |
| :----------------- | :----------------------------------------------------------- |
| 模型名         | Resnet50-Imagenet                                            |
| 下载链接                | [https://paddle-serving.bj.bcebos.com/imagenet-example/ResNet50_vd.tar.gz](https://paddle-serving.bj.bcebos.com/imagenet-example%2FResNet50_vd.tar.gz) |
| 客户端/服务端代码 | https://github.com/PaddlePaddle/Serving/tree/develop/python/examples/imagenet |
| 介绍        | 获得一张图片的图像语义表示              |



| Key                | Value                                                        |
| :----------------- | :----------------------------------------------------------- |
| 模型名       | Resnet101-Imagenet                                           |
| 下载链接                | https://paddle-serving.bj.bcebos.com/imagenet-example/ResNet101_vd.tar.gz |
| 客户端/服务端代码 | https://github.com/PaddlePaddle/Serving/tree/develop/python/examples/imagenet |
| 介绍      | 获得一张图片的图像语义表示              |



| Key                | Value                                                        |
| :----------------- | :----------------------------------------------------------- |
| 模型名        | CNN-IMDB                                                     |
| 下载链接                | https://paddle-serving.bj.bcebos.com/imdb-demo/imdb_model.tar.gz |
| 客户端/服务端代码 | https://github.com/PaddlePaddle/Serving/tree/develop/python/examples/imdb |
| 介绍       | 从一个中文语句获得类别及其概率           |



| Key                | Value                                                        |
| :----------------- | :----------------------------------------------------------- |
| 模型名         | LSTM-IMDB                                                    |
| 下载链接               | https://paddle-serving.bj.bcebos.com/imdb-demo/imdb_model.tar.gz |
| 客户端/服务端代码 | https://github.com/PaddlePaddle/Serving/tree/develop/python/examples/imdb |
| 介绍        | 从一个英文语句获得类别及其概率            |



| Key                | Value                                                        |
| :----------------- | :----------------------------------------------------------- |
| 模型名         | BOW-IMDB                                                     |
| 下载链接                | https://paddle-serving.bj.bcebos.com/imdb-demo/imdb_model.tar.gz |
| 客户端/服务端代码 | https://github.com/PaddlePaddle/Serving/tree/develop/python/examples/imdb |
| 介绍       | 从一个英文语句获得类别及其概率            |



| Key                | Value                                                        |
| :----------------- | :----------------------------------------------------------- |
| 模型名         | Jieba-LAC                                                    |
| 下载链接                | https://paddle-serving.bj.bcebos.com/lac/lac_model.tar.gz    |
| 客户端/服务端代码 | https://github.com/PaddlePaddle/Serving/tree/develop/python/examples/lac |
| 介绍       | 获取中文语句的分词                |



| Key                | Value                                                        |
| :----------------- | :----------------------------------------------------------- |
| 模型名         | DNN-CTR                                                      |
| 下载链接                | https://paddle-serving.bj.bcebos.com/criteo_ctr_example/criteo_ctr_demo_model.tar.gz                    |
| 客户端/服务端代码 | https://github.com/PaddlePaddle/Serving/tree/develop/python/examples/criteo_ctr |
| 介绍        | 从项目的特征向量中获得点击概率        |



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
- [常见问答](doc/deprecated/FAQ.md)

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
