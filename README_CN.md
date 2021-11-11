(简体中文|[English](./README.md))

<p align="center">
    <br>
<img src='doc/images/serving_logo.png' width = "600" height = "130">
    <br>
<p>

<p align="center">
    <br>
    <a href="https://travis-ci.com/PaddlePaddle/Serving">
        <img alt="Build Status" src="https://img.shields.io/travis/com/PaddlePaddle/Serving/develop?style=flat-square">
        <img alt="Docs" src="https://img.shields.io/badge/docs-中文文档-brightgreen?style=flat-square">
        <img alt="Release" src="https://img.shields.io/badge/release-0.7.0-blue?style=flat-square">
        <img alt="Python" src="https://img.shields.io/badge/python-3.6+-blue?style=flat-square">
        <img alt="License" src="https://img.shields.io/github/license/PaddlePaddle/Serving?color=blue&style=flat-square">
        <img alt="Forks" src="https://img.shields.io/github/forks/PaddlePaddle/Serving?color=yellow&style=flat-square">
        <img alt="Issues" src="https://img.shields.io/github/issues/PaddlePaddle/Serving?color=yellow&style=flat-square">
        <img alt="Contributors" src="https://img.shields.io/github/contributors/PaddlePaddle/Serving?color=orange&style=flat-square">
        <img alt="Community" src="https://img.shields.io/badge/join-Wechat,QQ,Slack-orange?style=flat-square">
    </a>
    <br>
<p>


Paddle Serving依托PaddlePaddle旨在帮助深度学习开发者提供高性能、灵活易用、可在云端部署的在线推理服务。Paddle Serving支持RESTful、gRPC、bRPC等多种协议，提供多种异构硬件和多种操作系统环境下推理解决方案，为深度学习开发者提供丰富的预训练模型示例。核心特性如下：

- 全面支持PaddlePaddle训练模型，通过[x2paddle](https://github.com/PaddlePaddle/X2Paddle)工具可快速将Caffe/TensorFlow/ONNX/PyTorch预测模型迁移到Paddle框架
- 基于高性能bRPC网络框架打造高吞吐、低延迟的推理服务。服务端支持HTTP、gRPC、bRPC等多种[协议](链接protocol文档)，并提供python、Java、C++多种语言SDK
- 支持x86(Intel) CPU、ARM CPU、Nvidia GPU、昆仑XPU等多种硬件上部署推理服务，提供[异构硬件部署环境和包](异构硬件文档链接)
- 基于有向无环图(DAG)的异步流水线高性能推理框架，具有[多模型组合]()、[异步调度]()、[并发推理]()、[动态批量]()、[多卡多流推理]()等特性，提供性能分析与优化指南.
- 提供[加密模型的服务部署](链接)，通过模型加密和服务鉴权机制保护模型安全。通过HTTPs安全网关实现安全请求校验
- 云端部署，支持docker和[Kubernetes云端部署](链接)
- 支持[Paddle预训练模型库](链接)，已支持PaddleOCR、PaddleClas、PaddleDetection、PaddleNLP、PaddleRec等套件，共计50+预训练模型示例
- 支持大规模稀疏参数模型分布式部署，具有多表、多分片、多副本、本地高频cache、可云端部署等特性

<br>

## 教程

***

- AIStudio教程-[Paddle Serving服务化部署框架](https://www.paddlepaddle.org.cn/tutorials/projectdetail/1555945)

- 视频教程-[深度学习服务化部署-以互联网应用为例](https://aistudio.baidu.com/aistudio/course/introduce/19084)
<p align="center">
    <img src="doc/images/demo.gif" width="700">
</p>

## 文档

***

### 部署
此章节引导您完成安装和部署步骤，强烈推荐使用Docker部署Paddle Serving，如您不使用docker，省略docker相关步骤。在云服务器上可以使用Kubernetes部署Paddle Serving。在异构硬件如ARM CPU、昆仑XPU上编译或使用Paddle Serving可以下面的文档。每天编译生成develop分支的最新开发包供开发者使用。
- [使用docker安装Paddle Serving](doc/Install_CN.md)
- [源码编译安装Paddle Serving](doc/COMPILE_CN.md)
- [在Kuberntes集群上部署Paddle Serving](doc/PADDLE_SERVING_ON_KUBERNETES.md)
- [部署Paddle Serving安全网关](doc/SERVING_AUTH_DOCKER.md)
- [在异构硬件部署Paddle Serving](doc/BAIDU_KUNLUN_XPU_SERVING_CN.md)
- [最新Wheel开发包](doc/LATEST_PACKAGES.md)(develop分支每日更新)

### 使用
安装Paddle Serving后，使用快速开始将引导您运行Serving示例的重要步骤，通过客户端程序发送推理请求并执行出推理结果。使用PaddleServing为您提供服务的第一步是模型保存接口，读取paddle模型文件生成模型参数配置文件(.prototxt)。配置和启动参数文件非常重要，详细介绍可使用的系统功能和配置方法。RESTful/gRPC/bRPC API指南文件介绍网络服务接口和使用规则。

Paddle Serving有2套服务框架，C++ Serving和Python Pipeline。C++ Serving使用C++语言开发，适用于高并发、高性能服务场景。Python Pipeline使用Python语言开发，侧重易用性和开发效率。分别介绍功能特性，性能分析和优化的方法

目前，Paddle Serving有3种开发语言的客户端SDK，每种SDK有多个的示例供参考。
- [快速开始](doc/QuickStart_CN.md)
- [保存用于Paddle Serving的模型](doc/SAVE_CN.md)
- [配置和启动参数说明](doc/SERVING_CONFIGURE.md)
- [RESTful/gRPC/bRPC API指南](doc/HTTP_SERVICE_CN.md)
- [低精度推理](doc/LOW_PRECISION_DEPLOYMENT_CN.md)
- [常见模型数据处理](doc/PROCESS_DATA.md)
- [C++ Serving]() 
  - [功能简介]()
  - [模型热加载](doc/HOT_LOADING_IN_SERVING_CN.md)
  - [A/B Test](doc/ABTEST_IN_PADDLE_SERVING_CN.md)
  - [性能优化指南]()
- [Python Pipeline]()
  - [功能简介]()
  - [性能优化指南]()
- [客户端SDK]()
  - [Python SDK](doc/PYTHON_SDK_CN.md)
  - [JAVA SDK](doc/JAVA_SDK_CN.md)
  - [C++ SDK](doc/C++_SDK_CN.md)

### 开发者
作为Paddle Serving开发者，我们深入了解的架构设计，扩展自定义OP，变长数据处理和性能指标。
- [C++ Serving架构设计](doc/C++DESIGN_CN)
- [Python Pipeline架构设计](doc/PIPELINE_SERVING_CN.md)
- [自定义OP](doc/NEW_OPERATOR_CN.md)
- [变长数据(LOD)处理](doc/LOD_CN.md)
- [性能指标](doc/BENCHMARKING_GPU.md)

### FAQ
- [常见问答](doc/FAQ.md)

<br>

## 模型库

***
Paddle Serving已全面Paddle训练模型，并实现多个Paddle模型套件服务化部署，包括图像分类、物体检测、语言文本识别、中文词性、情感分析、内容推荐等多种类型示例，以及Paddle全链条项目，共计42个模型。
<center class="half">

| PaddleOCR | PaddleDetection | PaddleClas | PaddleSeg | PaddleRec | Paddle NLP | 
| :----:  | :----: | :----: | :----: | :----: | :----: | 
| 8 | 12 | 13 | 2 | 3 | 4 | 

</center>
更多模型示例参考Repo，可进入

<center class="half">
  <img src="https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.3/doc/imgs_results/PP-OCRv2/PP-OCRv2-pic003.jpg?raw=true" width="280"/> <img src="https://github.com/PaddlePaddle/PaddleDetection/raw/release/2.3/docs/images/road554.png" width="160"/>
  <img src="https://github.com/PaddlePaddle/PaddleClas/raw/release/2.3/docs/images/recognition.gif" width="213"/>
</center>


## 社区

***
想要同开发者和其他用户沟通吗？欢迎加入我们，通过如下方式加入社群

### 微信
- 微信用户请扫码

### QQ用户
- 飞桨推理部署交流群(群号：696965088)

### Slack
- [Slack channel](https://paddleserving.slack.com/archives/CUBPKHKMJ)


### 贡献代码

如果您想为Paddle Serving贡献代码，请参考 [Contribution Guidelines](doc/CONTRIBUTE.md)

- 特别感谢 [@BeyondYourself](https://github.com/BeyondYourself) 提供grpc教程，更新FAQ教程，整理文件目录。
- 特别感谢 [@mcl-stone](https://github.com/mcl-stone) 提供faster rcnn benchmark脚本
- 特别感谢 [@cg82616424](https://github.com/cg82616424) 提供unet benchmark脚本和修改部分注释错误
- 特别感谢 [@cuicheng01](https://github.com/cuicheng01) 提供PaddleClas的11个模型

### 反馈

如有任何反馈或是bug，请在 [GitHub Issue](https://github.com/PaddlePaddle/Serving/issues)提交

### License

[Apache 2.0 License](https://github.com/PaddlePaddle/Serving/blob/develop/LICENSE)
