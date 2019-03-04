[TOC]

# 概述
PaddlePaddle是公司开源的机器学习框架，广泛支持各种深度学习模型的定制化开发;
Paddle cloud是基于PaddlePaddle框架实现的一整套云平台，对外提供全流程的AI开发平台，对内托管集团内各产品线的机器学习云服务。

Paddle serving是Paddle cloud的在线预测部分，与Paddle cloud模型训练环节无缝衔接，对外提供机器学习预测共有云服务，对内为公司各业务线提供统一的模型预测开发框架和云服务。

# Getting Started
## 运行示例
说明：Imagenet图像分类模型，默认采用CPU模式（GPU模式请修改BCLOUD配置项，并用Dockerfile构建运行环境，[Docker部署请参考Wiki](http://agroup.baidu.com/share/md/044f552e866f4078900be503784e2468)）。

Step1：启动Server端：
```shell
git clone ssh://icode.baidu.com:8235/baidu/paddle-serving/serving  ~/my_paddle_serving/baidu/paddle-serving/serving && cd ~/my_paddle_serving/baidu/paddle-serving/serving && bcloud build &&  ./output/bin/image_class &
```

Step2：启动Client端：
```shell
git clone ssh://icode.baidu.com:8235/baidu/paddle-serving/sdk-cpp ~/my_paddle_serving/baidu/paddle-serving/sdk-cpp && cd ~/my_paddle_serving/baidu/paddle-serving/sdk-cpp && bcloud build &&  ./output/bin/ximage && pkill image_class
```

## 示例说明
### 预测接口定义
```c++
syntax="proto2";
package baidu.paddle_serving.predictor.image_class;
option cc_generic_services = true;

// x-image request相关（批量接口）
message XImageReqInstance {
    required bytes image_binary = 1;
    required uint32 image_length = 2;
};

message Request {
    repeated XImageReqInstance instances = 1;
};

// x-image response相关（批量接口）
message DensePrediction {
    repeated float categories = 1;
};

message ClassResponse {
    repeated DensePrediction predictions = 1;
};

message XImageResInstance {
    required string response_json = 1;
};

message Response {
    // Each json string is serialized from ClassResponse
    repeated XImageResInstance predictions = 1;
};

// Service/method相关
service ImageClassifyService {
    rpc inference(Request) returns (Response);
    rpc debug(Request) returns (Response);
};
```
### Server端实现
用户只需定制或配置以下三类信息的实现，即可快速搭建完整的Paddle-Serving预测模块。

#### 接口改造([proto目录](http://icode.baidu.com/repos/baidu/paddle-serving/serving/tree/master:proto/))
Server端需对预测接口作如下修改即可：
```c++
// 改动1：依赖paddle-serving option接口文件
import "pds_option.proto";
...
service ClassService {
    rpc inference(Request) returns (Response);
    rpc debug(Request) returns (Response);
    // 改动2：打开generate_impl开关(以支持配置驱动)
    option (pds.options).generate_impl = true;
};
```

#### 示例配置([conf目录](http://icode.baidu.com/repos/baidu/paddle-serving/serving/tree/master:conf/))
- gflags配置项

| name | 默认值 | 含义 |
|------|--------|------|
| workflow_path | ./conf | workflow配置目录名 |
|workflow_file|workflow.conf|workflow配置文件名|
|inferservice_path|./conf|service配置目录名|
|inferservice_file|service.conf|service配置文件名|
|logger_path|./conf|日志配置目录名|
|logger_file|log.conf|日志配置文件名|
|resource_path|./conf|资源管理器目录名|
|resource_file|resource.conf|资源管理器文件名|
|reload_interval_s|10|重载线程间隔时间(s)|

- 配置文件实例(Image图像分类demo)
```shell
# >>> service.conf
[@Service]
name: ImageClassifyService
@workflow: workflow_image_classification

# >>> workflow.conf
[@Workflow]
name: workflow_image_classification
path: ./conf
file: imagec_dag.conf

# >>> imagec_dag.conf
workflow_type: Sequence
[@Node]
name: image_reader_op
type: ImageReaderOp

[@Node]
name: image_classify_op
type: ImageClassifyOp
[.@Depend]
name: image_reader_op
mode: RO

[@Node]
name: write_json_op
type: WriteJsonOp
[.@Depend]
name: image_classify_op
mode: RO

# >>> resource.conf
model_manager_path: ./conf
model_manager_file: model_toolkit.conf
```

#### 定制Op算子([op目录](http://icode.baidu.com/repos/baidu/paddle-serving/serving/tree/master:op/))
- 预处理算子(ImageReaderOp)：从Request中读取图像字节流，通过opencv解码，填充tensor对象并输出到channel；
- 预测调用算子(ImageClassifyOp)：从ImageReaderOp的channel获得输入tensor，临时申请输出tensor，调用ModelToolkit进行预测，并将输出tensor写入channel
- 后处理算子(WriteJsonOp)：从ImageClassifyop的channel获得输出tensor，将其序列化为json字符串，写入作为rpc的output；

### Client端实现
用户只需定制或配置以下三类信息，即可方便的接入预估请求，并在本地配置多套服务连接：

#### 接口改造([proto目录](http://icode.baidu.com/repos/baidu/paddle-serving/sdk-cpp/tree/master:proto))
Client端接口只需对预测接口作如下修改即可：
```c++
// 改动1：依赖paddle-serving option接口文件
import "pds_option.proto";
...
service ImageClassifyService {
    rpc inference(Request) returns (Response);
    rpc debug(Request) returns (Response);
    // 改动2：打开generate_stub开关(以支持配置驱动)
    option (pds.options).generate_stub = true;
};
```

#### 连接配置([conf目录](http://icode.baidu.com/repos/baidu/paddle-serving/sdk-cpp/tree/master:conf))
```shell
# predictions.conf
## 默认配置共享
[DefaultVariantInfo]
Tag : default
[.Connection]
ConnectTimeoutMicroSec : 200
ReadTimeoutMicroSec : 2000
WriteTimeoutMicroSec : 500
ConnectRetryCount : 2
MaxConnectionPerHost : 100
HedgeRequestTimeoutMicroSec : -1
HedgeFetchRetryCount : 2
BnsReloadIntervalSeconds : 10
ConnectionType : pooled
[.NamingInfo]
ClusterFilterStrategy : Default
LoadBalanceStrategy : la
[.RpcParameter]
# 0-NONE, 1-SNAPPY, 2-GZIP, 3-ZLIB, 4-LZ4
CompressType : 0
Protocol : baidu_std
MaxChannelPerRequest : 3

[@Predictor]
name : ximage
service_name : baidu.paddle_serving.predictor.image_class.ImageClassifyService
endpoint_router : WeightedRandomRender
[.WeightedRandomRender]
VariantWeightList : 30|70 # 30% vs 70% pvs
[.@VariantInfo]
Tag : var1  # 变体版本标识，提供上游辨识
[..NamingInfo]
Cluster : list://127.0.0.1:8010
[.@VariantInfo]
Tag : var2
[..NamingInfo]
Cluster : list://127.0.0.1:8011
```

#### 请求逻辑([demo/ximage.cpp](http://icode.baidu.com/repos/baidu/paddle-serving/sdk-cpp/blob/master:demo/ximage.cpp))
```c++
// 进程级初始化
assert(PredictorAPI::instance().create("./conf/predictions.conf") == 0);
// 线程级预测调用：
Request req;
// fill request
// ...
Response res;
Predictor* ximage = PredictorAPI::instance().fetch_predictor("ximage");
assert(ximage != NULL);
ximage->inference(req, res);
// parse response
// ...
assert(PredictorAPI::instance().free_predictor(ximage) == 0);

// 进程级销毁
assert(PredictorAPI::instance().destroy() == 0);
```

## 凤巢协议兼容
Paddle Serving由凤巢观星框架发展而来，而之前框架的通信协议是nshead+compack+idl，为方便新老接口的兼容，Paddle Serving的server和client均支持向后兼容：
- 老API访问新Server，为适配老观星客户端数据包格式，新Server需通过mcpack2pb生成能解析idl格式的pb对象，详见：[wtitleq server实现](http://icode.baidu.com/repos/baidu/paddle-serving/lr-model/tree/master)
- 新SDK访问老Server，为能够访问老观星server服务，SDK需通过mcpack2pb插件生成基于idl格式的序列化逻辑；详见：[wtitleq api实现](http://icode.baidu.com/repos/baidu/infinite-inference/as-wtitleq-demo/tree/master)。

凤巢广告拆包支持：Paddle Serving的SDK-Cpp为用户提供了简单易用的拆包功能，通过修改proto/conf文件开启：
```c++
// interface.proto文件
message PredictorRequest {
    message AdvRequest {
        // 广告级别字段
        repeated uint32 ideaid = 1;
        repeated string title = 2;
    }
    
    // query级别字段
    required uint64 sid = 1;
    required string query = 2;
    // ...
    
    // 广告级别字段
    repeated AdvRequest advs = 3 [(pds.pack_on)=true]; // 改动1：对advs字段进行拆包
} 

// ...

service WtitleqService {
    rpc ...
    rpc ...
    option (pds.options).package_size = 10; // 改动2：限制单包大小
}
```
[wtitleq sdk的proto实例](http://icode.baidu.com/repos/baidu/infinite-inference/as-wtitleq-demo/blob/master:proto/predictor_api.proto)。

```bash
# predictions.conf文件
[@Predictor]
# ...
[.@VariantInfo]
#...
[..RpcParameter]
Protocol : itp # 改动3：修改rpc请求参数为itp协议

```
[wtitleq sdk的conf实例](http://icode.baidu.com/repos/baidu/infinite-inference/as-wtitleq-demo/blob/master:conf/predictors.conf)。

# 框架简介

![图片](http://agroup-bos.cdn.bcebos.com/63a5076471e96a08124b89101e12c1a0ec7b642a)

- 基础框架：屏蔽一个RPC服务所需的所有元素，让用户只关注自己的业务算子的开发；
- 业务框架：基于Protobuf定制请求接口，基于有限DAG定制业务逻辑，并行化调度；
- 模型框架：CPU/FPGA/GPU等硬件异构，多模型间异步优先级调度，新引擎灵活扩展，配置化驱动；
- 用户接口：搭建服务=定义proto文件+实现/复用Op+撰写配置，支持sdk/http请求；

## 名词解释
- 预测引擎：对PaddlePaddle/Abacus/Tensorflow等各种推理计算Lib的封装，屏蔽预测模型动态Reload细节，对上层暴露统一的预测接口；
- 预测模型：由离线训练框架生成、在线预测引擎加载的数据文件或目录，以PaddleFluid模型为例，通常包括拓扑文件和参数文件；
- Op 算子：Paddle-serving对在线(预处理/后处理等)业务逻辑的最小粒度封装，框架提供OpWithChannel和OpWithChannelAndConf这两种常用的Op基类；框架默认实现通用Op算子；
- Node：由某个Op算子类结合参数配置组成的Op算子实例，也是Workflow中的一个执行单元；
- DAG/Workflow：由若干个相互依赖的Node组成，每个Node均可通过特定接口获得Request对象，节点Op通过依赖关系获得其前置Op的输出对象，最后一个Node的输出默认就是Response对象；
- Service：对一次pv的请求封装，可配置若干条Workflow，彼此之间复用当前PV的Request对象，然后各自并行/串行执行，最后将Response写入对应的输出slot中；一个Paddle-serving进程可配置多套Service接口，上游根据ServiceName决定当前访问的Service接口。

![图片](http://agroup-bos.cdn.bcebos.com/2e5e3cdcc9426d16e2090e64e7d33098ae5ad826)

## 主要功能

Paddle serving框架为策略工程师提供以下三层面的功能性扩展：

### 模型
- 预测引擎：集成PaddlePaddle、Abacus、Tensorrt、Anakin、Tensorflow等常用机器学习框架的预测Lib；
- 模型种类：支持PaddlePaddle(V1、V2、Fluid)、TensorrtUFF、Anakin、Tensorflow、Caffe等常见模型格式；
- 用户接口：支持模型加载、重载的配置化驱动，不同种类模型的预测接口完全一致；
- 模型调度：支持基于异步线程模型的多模型预估调度，实现异构资源的优先级调度；

### 业务
- 预测流程：通过有限DAG图描述一次预测从Request到Response的业务流程，节点Node是一个最小逻辑单元——OP；
- 预测逻辑：框架封装常用预处理、预测计算、后处理等常用OP，用户通过自定义OP算子实现特化处理逻辑；

### 服务

- RPC：底层通过Baidu-rpc封装网络交互，Server端可配置化启动多个独立Service，框架会搜集Service粒度的详细业务指标，并按照BVar接口对接到Noah等监控平台；
- SDK：基于Baidu-rpc的client进行封装，提供多下游连接管理、可扩展路由策略、可定制参数实验、自动分包等机制，支持同步、半同步、纯异步等交互模式，以及多种兼容协议，所有连接策略均通过配置驱动

# 平台简介
![图片](http://agroup-bos.cdn.bcebos.com/42a0e34a7c6b36976e3932639209fd823d8f25e0)

- [运维API](http://agroup.baidu.com/share/md/e582f543fb574e9b92445286955a976d)
- [预测API](http://agroup.baidu.com/share/md/eb91a51739514319844ceccdb331564c)

## 名词解释
- 用户（User）：云平台注册用户，可基于平台Dashboard对账户下的端点信息进行增、删、查、改；
- 端点（Endpoit）：对一个预测需求的逻辑抽象，通常包含一到多个服务变体，以方便多版本模型管理；
- 变体（Variant）：一套同质化的Paddle-serving集群服务，每个实例起一个Paddle-serving进程；
- 实验（A/B Test）：支持变体实验和参数化实验两种模式，变体实验根据Endpoint所属变体流量百分比实现流量随机抽样；参数化实验通过对pv绑定实验参数、由Paddle-serving进程解析参数、选择不同的代码分支进行实验；

## 主要功能
在公有云落地场景为Infinite（天衍）云平台，主要为策略工程师提供以下三方面的全流程托管：
- 统一接入代理：提供代理服务，通过zk和云平台实时同步元信息，支持多模型版本管理和A/B测试路由策略，提供统一入口和标准预测API；
- 自动化部署：对接K8S/Opera等常见PaaS部署平台，支持服务的一键部署、回滚、下线等运维操作，支持endpoint/variant/model等维度的资源管理；
- 可视化运维：对接console、notebook、dashboard等前端工具和页面，满足可视化运维需求；

# 设计文档
- [总体设计文档](http://agroup.baidu.com/paddleserving/view/office/895070)
- [框架详设文档](http://agroup.baidu.com:8964/static/a3/e40876e464ba08ae5de14aa7710cf326456751.pdf?filename=PaddleServing%E6%9C%8D%E5%8A%A1%E6%A1%86%E6%9E%B6%E8%AF%A6%E7%BB%86%E8%AE%BE%E8%AE%A1%E6%96%87%E6%A1%A3v0_1.pdf)
- [平台详设文档](http://agroup.baidu.com/share/office/042a0941579e49adb8c255c8b5e92d51)

# FAQ
1. 如何修改端口配置？
- 使用该框架搭建的服务需要申请一个端口，可以通过以下方式修改端口号：
- 如果在inferservice_file里指定了port:xxx，那么就去申请该端口号；
- 否则，如果在gflags.conf里指定了--port:xxx，那就去申请该端口号；
- 否则，使用程序里指定的默认端口号：8010。
2. 如何在部署的时候配置动态端口？
- 如果使用FCCI部署协议(凤巢检索端内部的部署协议)，需要(1)通过inferservice_file指定端口号；(2)修改[Rakefile.opera](http://wiki.baidu.com/pages/viewpage.action?pageId=399979183#id-%E4%BB%8E%E9%9B%B6%E5%BC%80%E5%A7%8B%E5%86%99production-%E7%BC%96%E5%86%99Rakefile)的dynamic_port_config配置
- `@dynamic_port_config为动态端口配置，向Opera申请名为:name的动态端口，其端口号会被写到:conf文件中的:target配置项。`例子如下：
```
@dynamic_port_config = [
    {:name => 'main', :conf => 'framework/service.conf', :target => 'port'}, // 部署时自动向Opera申请端口，服务将会监听这个端口
    {:name => 'main', :conf => 'predictor_valid.conf', :target => 'port'}, // valid工具向这个端口发送测试请求，确保服务已正常启动
]  
```
