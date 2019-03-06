
# 概述
PaddlePaddle是公司开源的机器学习框架，广泛支持各种深度学习模型的定制化开发; Paddle serving是Paddle的在线预测部分，与Paddle模型训练环节无缝衔接，提供机器学习预测云服务。

# 框架简介

![图片](https://paddle-serving.bj.bcebos.com/doc/framework.png)

- 基础框架：屏蔽一个RPC服务所需的所有元素，让用户只关注自己的业务算子的开发；
- 业务框架：基于Protobuf定制请求接口，基于有限DAG定制业务逻辑，并行化调度；
- 模型框架：CPU/FPGA/GPU等硬件异构，多模型间异步优先级调度，新引擎灵活扩展，配置化驱动；
- 用户接口：搭建服务=定义proto文件+实现/复用Op+撰写配置，支持sdk/http请求；

## 主要功能

Paddle serving框架为策略工程师提供以下三层面的功能性扩展：

### 模型
- 预测引擎：集成PaddlePaddle深度学习框架的预测Lib；
- 模型种类：支持Paddle Fluid模型格式；
- 用户接口：支持模型加载、重载的配置化驱动，不同种类模型的预测接口完全一致；
- 模型调度：支持基于异步线程模型的多模型预估调度，实现异构资源的优先级调度；

### 业务
- 预测流程：通过有限DAG图描述一次预测从Request到Response的业务流程，节点Node是一个最小逻辑单元——OP；
- 预测逻辑：框架封装常用预处理、预测计算、后处理等常用OP，用户通过自定义OP算子实现特化处理逻辑；

### 服务

- RPC：底层通过Baidu-rpc封装网络交互，Server端可配置化启动多个独立Service，框架会搜集Service粒度的详细业务指标，并按照BVar接口对接到Noah等监控平台；
- SDK：基于Baidu-rpc的client进行封装，提供多下游连接管理、可扩展路由策略、可定制参数实验、自动分包等机制，支持同步、半同步、纯异步等交互模式，以及多种兼容协议，所有连接策略均通过配置驱动


# 文档

[设计文档](doc/DESIGN.md)

[从零开始写一个预测服务](doc/CREATING.md)

[编译安装](doc/INSTALL.md)

[FAQ](doc/FAQ.md)
