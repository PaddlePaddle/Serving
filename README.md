[TOC]

# 概述
PaddlePaddle是公司开源的机器学习框架，广泛支持各种深度学习模型的定制化开发; Paddle serving是Paddle的在线预测部分，与Paddle模型训练环节无缝衔接，提供机器学习预测云服务。

# 框架简介

![图片](http://agroup-bos.cdn.bcebos.com/63a5076471e96a08124b89101e12c1a0ec7b642a)

- 基础框架：屏蔽一个RPC服务所需的所有元素，让用户只关注自己的业务算子的开发；
- 业务框架：基于Protobuf定制请求接口，基于有限DAG定制业务逻辑，并行化调度；
- 模型框架：CPU/FPGA/GPU等硬件异构，多模型间异步优先级调度，新引擎灵活扩展，配置化驱动；
- 用户接口：搭建服务=定义proto文件+实现/复用Op+撰写配置，支持sdk/http请求；

## 名词解释
- 预测引擎：对PaddlePaddle预测Lib的封装，屏蔽预测模型动态Reload细节，对上层暴露统一的预测接口；
- 预测模型：由离线训练框架生成、在线预测引擎加载的数据文件或目录，以PaddleFluid模型为例，通常包括拓扑文件和参数文件；
- Op 算子：Paddle-serving对在线(预处理/后处理等)业务逻辑的最小粒度封装，框架提供OpWithChannel和OpWithChannelAndConf这两种常用的Op基类；框架默认实现通用Op算子；
- Node：由某个Op算子类结合参数配置组成的Op算子实例，也是Workflow中的一个执行单元；
- DAG/Workflow：由若干个相互依赖的Node组成，每个Node均可通过特定接口获得Request对象，节点Op通过依赖关系获得其前置Op的输出对象，最后一个Node的输出默认就是Response对象；
- Service：对一次pv的请求封装，可配置若干条Workflow，彼此之间复用当前PV的Request对象，然后各自并行/串行执行，最后将Response写入对应的输出slot中；一个Paddle-serving进程可配置多套Service接口，上游根据ServiceName决定当前访问的Service接口。

![图片](http://agroup-bos.cdn.bcebos.com/2e5e3cdcc9426d16e2090e64e7d33098ae5ad826)

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

## 名词解释
- 端点（Endpoit）：对一个预测需求的逻辑抽象，通常包含一到多个服务变体，以方便多版本模型管理；
- 变体（Variant）：一套同质化的Paddle-serving集群服务，每个实例起一个Paddle-serving进程；

# 设计文档

# FAQ
1. 如何修改端口配置？
- 使用该框架搭建的服务需要申请一个端口，可以通过以下方式修改端口号：
- 如果在inferservice_file里指定了port:xxx，那么就去申请该端口号；
- 否则，如果在gflags.conf里指定了--port:xxx，那就去申请该端口号；
- 否则，使用程序里指定的默认端口号：8010。
