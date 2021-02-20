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



- [动机](./README_CN.md#动机)
- [教程](./README_CN.md#教程)
- [安装](./README_CN.md#安装)
- [快速开始示例](./README_CN.md#快速开始示例)
- [文档](README_CN.md#文档)
- [社区](README_CN.md#社区)

<h2 align="center">动机</h2>

Paddle Serving 旨在帮助深度学习开发者轻易部署在线预测服务。 **本项目目标**: 当用户使用 [Paddle](https://github.com/PaddlePaddle/Paddle) 训练了一个深度神经网络，就同时拥有了该模型的预测服务。

- 任何经过[PaddlePaddle](https://github.com/paddlepaddle/paddle)训练的模型，都可以经过直接保存或是[模型转换接口](./doc/SAVE_CN.md)，用于Paddle Serving在线部署。
- 支持[多模型串联服务部署](./doc/PIPELINE_SERVING_CN.md), 同时提供Rest接口和RPC接口以满足您的需求，[Pipeline示例](./python/examples/pipeline)。
- 支持Paddle生态的各大模型库, 例如[PaddleDetection](./python/examples/detection)，[PaddleOCR](./python/examples/ocr)，[PaddleRec](https://github.com/PaddlePaddle/PaddleRec/tree/master/tools/recserving/movie_recommender)。
- 提供丰富多彩的前后处理，方便用户在训练、部署等各阶段复用相关代码，弥合AI开发者和应用开发者之间的鸿沟，详情参考[模型示例](./python/examples/)。

<p align="center">
    <img src="doc/demo.gif" width="700">
</p>

<h2 align="center">教程</h2>

Paddle Serving开发者为您提供了简单易用的[AIStudio教程-Paddle Serving服务化部署框架](https://aistudio.baidu.com/aistudio/projectdetail/1550674)

教程提供了如下内容

<ul>
<li>Paddle Serving环境安装</li>
  <ul>
    <li>Docker镜像启动方式
    <li>pip安装Paddle Serving
  </ul>
<li>快速体验部署在线推理服务</li>
<li>部署在线推理服务进阶流程</li>
  <ul>
    <li>获取可用于部署在线服务的模型</li>
    <li>启动推理服务</li>
  </ul>
<li>Paddle Serving在线部署实例</li>
  <ul>
    <li>使用Paddle Serving部署图像检测服务</li>
    <li>使用Paddle Serving部署OCR Pipeline在线服务</li>
  </ul>
</ul>


<h2 align="center">安装</h2>

**强烈建议**您在**Docker内构建**Paddle Serving，请查看[如何在Docker中运行PaddleServing](doc/RUN_IN_DOCKER_CN.md)。更多镜像请查看[Docker镜像列表](doc/DOCKER_IMAGES_CN.md)。

**提示**：目前paddlepaddle 2.0版本的默认GPU环境是Cuda 10.2，因此GPU Docker的示例代码以Cuda 10.2为准。镜像和pip安装包也提供了其余GPU环境，用户如果使用其他环境，需要仔细甄别并选择合适的版本。

```
# 启动 CPU Docker
docker pull registry.baidubce.com/paddlepaddle/serving:0.5.0-devel
docker run -p 9292:9292 --name test -dit registry.baidubce.com/paddlepaddle/serving:0.5.0-devel
docker exec -it test bash
git clone https://github.com/PaddlePaddle/Serving
```
```
# 启动 GPU Docker
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
# 其他GPU环境需要确认环境再选择执行哪一条
pip install paddle-serving-server-gpu==0.5.0.post9 # GPU with CUDA9.0 
pip install paddle-serving-server-gpu==0.5.0.post10 # GPU with CUDA10.0 
pip install paddle-serving-server-gpu==0.5.0.post101 # GPU with CUDA10.1 + TensorRT6
pip install paddle-serving-server-gpu==0.5.0.post11 # GPU with CUDA10.1 + TensorRT7
```

您可能需要使用国内镜像源（例如清华源, 在pip命令中添加`-i https://pypi.tuna.tsinghua.edu.cn/simple`）来加速下载。

如果需要使用develop分支编译的安装包，请从[最新安装包列表](./doc/LATEST_PACKAGES.md)中获取下载地址进行下载，使用`pip install`命令进行安装。如果您想自行编译，请参照[Paddle Serving编译文档](./doc/COMPILE_CN.md)

paddle-serving-server和paddle-serving-server-gpu安装包支持Centos 6/7, Ubuntu 16/18和Windows 10。

paddle-serving-client和paddle-serving-app安装包支持Linux和Windows，其中paddle-serving-client仅支持python2.7/3.5/3.6/3.7/3.8。

推荐安装2.0.0及以上版本的paddle

```
# CPU环境请执行
pip install paddlepaddle==2.0.0

# GPU Cuda10.2环境请执行
pip install paddlepaddle-gpu==2.0.0
```

**注意**： 如果您的Cuda版本不是10.2，请勿直接执行上述命令，需要参考[Paddle官方文档-多版本whl包列表](https://www.paddlepaddle.org.cn/documentation/docs/zh/install/Tables.html#whl-release)

选择相应的GPU环境的url链接并进行安装，例如Cuda 9.0的Python2.7用户，请选择表格当中的`cp27-cp27mu`和`cuda9.0_cudnn7-mkl`对应的url，复制下来并执行
```
pip install https://paddle-wheel.bj.bcebos.com/2.0.0-gpu-cuda9-cudnn7-mkl/paddlepaddle_gpu-2.0.0.post90-cp27-cp27mu-linux_x86_64.whl
```
如果是其他环境和Python版本，请在表格中找到对应的链接并用pip安装。

对于**Windows 10 用户**，请参考文档[Windows平台使用Paddle Serving指导](./doc/WINDOWS_TUTORIAL_CN.md)。


<h2 align="center">快速开始示例</h2>

这个快速开始示例主要是为了给那些已经有一个要部署的模型的用户准备的，而且我们也提供了一个可以用来部署的模型。如果您想知道如何从离线训练到在线服务走完全流程，请参考前文的AiStudio教程。

<h3 align="center">波士顿房价预测</h3>

进入到Serving的git目录下，进入到`fit_a_line`例子
``` shell
cd Serving/python/examples/fit_a_line
sh get_data.sh
```

Paddle Serving 为用户提供了基于 HTTP 和 RPC 的服务

<h3 align="center">RPC服务</h3>

用户还可以使用`paddle_serving_server.serve`启动RPC服务。 尽管用户需要基于Paddle Serving的python客户端API进行一些开发，但是RPC服务通常比HTTP服务更快。需要指出的是这里我们没有指定`--name`。

``` shell
python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9292
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
| `use_trt` (Only for Cuda>=10.1 version) | - | - | Run inference with TensorRT  |
| `use_lite` (Only for ARM) | - | - | Run PaddleLite inference |
| `use_xpu` (Only for ARM+XPU) | - | - | Run PaddleLite XPU inference |

</center>

``` python
# A user can visit rpc service through paddle_serving_client API
from paddle_serving_client import Client

client = Client()
client.load_client_config("uci_housing_client/serving_client_conf.prototxt")
client.connect(["127.0.0.1:9292"])
data = [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727,
        -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]
fetch_map = client.predict(feed={"x": np.array(data).reshape(1,13,1)}, fetch=["price"])
print(fetch_map)

```
在这里，`client.predict`函数具有两个参数。 `feed`是带有模型输入变量别名和值的`python dict`。 `fetch`被要从服务器返回的预测变量赋值。 在该示例中，在训练过程中保存可服务模型时，被赋值的tensor名为`"x"`和`"price"`。

<h3 align="center">HTTP服务</h3>
用户也可以将数据格式处理逻辑放在服务器端进行，这样就可以直接用curl去访问服务，参考如下案例，在目录`python/examples/fit_a_line`

```
python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9292 --name uci
```
客户端输入
```
curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"x": [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]}], "fetch":["price"]}' http://127.0.0.1:9292/uci/prediction
```
返回结果
```
{"result":{"price":[[18.901151657104492]]}}
```

<h2 align="center">Paddle Serving的核心功能</h2>

- 与Paddle训练紧密连接，绝大部分Paddle模型可以 **一键部署**.
- 支持 **工业级的服务能力** 例如模型管理，在线加载，在线A/B测试等.
- 支持 **分布式键值对索引** 助力于大规模稀疏特征作为模型输入.
- 支持客户端和服务端之间 **高并发和高效通信**.
- 支持 **多种编程语言** 开发客户端，例如Golang，C++和Python.

<h2 align="center">文档</h2>

### 新手教程
- [怎样保存用于Paddle Serving的模型？](doc/SAVE_CN.md)
- [十分钟构建Bert-As-Service](doc/BERT_10_MINS_CN.md)
- [Paddle Serving示例合辑](python/examples)

### 开发者教程
- [如何开发一个新的Web Service?](doc/NEW_WEB_SERVICE_CN.md)
- [如何编译PaddleServing?](doc/COMPILE_CN.md)
- [如何开发Pipeline?](doc/PIPELINE_SERVING_CN.md)
- [如何使用uWSGI部署Web Service](doc/UWSGI_DEPLOY_CN.md)
- [如何实现模型文件热加载](doc/HOT_LOADING_IN_SERVING_CN.md)

### 关于Paddle Serving性能
- [如何测试Paddle Serving性能？](python/examples/util/)
- [如何优化性能?](doc/PERFORMANCE_OPTIM_CN.md)
- [在一张GPU上启动多个预测服务](doc/MULTI_SERVICE_ON_ONE_GPU_CN.md)
- [CPU版Benchmarks](doc/BENCHMARKING.md)
- [GPU版Benchmarks](doc/GPU_BENCHMARKING.md)

### 设计文档
- [Paddle Serving设计文档](doc/DESIGN_DOC_CN.md)

### FAQ
- [常见问答](doc/FAQ.md)

<h2 align="center">社区</h2>

### Slack

想要同开发者和其他用户沟通吗？欢迎加入我们的 [Slack channel](https://paddleserving.slack.com/archives/CUBPKHKMJ)

### 贡献代码

如果您想为Paddle Serving贡献代码，请参考 [Contribution Guidelines](doc/CONTRIBUTE.md)

- 特别感谢 [@BeyondYourself](https://github.com/BeyondYourself) 提供grpc教程，更新FAQ教程，整理文件目录。
- 特别感谢 [@mcl-stone](https://github.com/mcl-stone) 提供faster rcnn benchmark脚本
- 特别感谢 [@cg82616424](https://github.com/cg82616424) 提供unet benchmark脚本和修改部分注释错误

### 反馈

如有任何反馈或是bug，请在 [GitHub Issue](https://github.com/PaddlePaddle/Serving/issues)提交

### License

[Apache 2.0 License](https://github.com/PaddlePaddle/Serving/blob/develop/LICENSE)
