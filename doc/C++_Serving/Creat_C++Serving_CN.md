# 从零开始写一个预测服务

## 1. 示例说明

图像分类是根据图像的语义信息将不同类别图像区分开来，是计算机视觉中重要的基本问题，也是图像检测、图像分割、物体跟踪、行为分析等其他高层视觉任务的基础。图像分类在很多领域有广泛应用，包括安防领域的人脸识别和智能视频分析等，交通领域的交通场景识别，互联网领域基于内容的图像检索和相册自动归类，医学领域的图像识别等。

本文接下来以图像分类任务为例，介绍从零搭建一个模型预测服务的步骤。

**本文件仅供参考，部分接口内容已经变动**


## 2. Serving端

### 2.1 定义预测接口

**添加文件：serving/proto/image_class.proto**
Paddle Serving服务端与客户端通过brpc进行通信，通信协议和格式可以自定，我们选择baidu_std协议。这是一种以protobuf为基本数据交换格式的协议，其说明可参考[BRPC文档: baidu_std](https://github.com/apache/incubator-brpc/blob/master/docs/cn/baidu_std.md)。


我们编写图像分类任务预测接口的protobuf如下：

```c++
syntax="proto2";
import "pds_option.proto";
import "builtin_format.proto";
package baidu.paddle_serving.predictor.image_classification;
option cc_generic_services = true;

message ClassifyResponse {
  repeated baidu.paddle_serving.predictor.format.DensePrediction predictions = 1;
};

message Request {
  repeated baidu.paddle_serving.predictor.format.XImageReqInstance instances = 1;
};

message Response {
  // Each json string is serialized from ClassifyResponse predictions
  repeated baidu.paddle_serving.predictor.format.XImageResInstance predictions = 1;
};

service ImageClassifyService {
  rpc inference(Request) returns (Response);
  rpc debug(Request) returns (Response);
  option (pds.options).generate_impl = true;
};
```

其中：
`service ImageClassifiyService`定义一个RPC Service，并声明2个RPC接口：`inference`和`debug`，分别接受`Reqeust`类型请求参数，并返回`Response`类型结果。

`DensePrediction`, `XImageReqInstance`和`XImageResInstance`类型的消息分别在其他.proto文件中定义，因此要通过`import 'builtin_format.proto'`语句将需要的类型引入。

`generate_impl = true`: 告诉protobuf编译器，生成RPC service的实现 (在client端，此处为`generate_stub = true`，告诉protobuf编译器生成RPC的stub)

### 2.2 Server端实现

图像分类任务的处理，设计分为3个阶段，对应3个OP
- 读请求：从Request消息解出请求样例数据
- 调用Paddle预测lib的接口，对样例进行预测，并保存
- 预测结果写到Response

此后，框架将负责将Response回传给client端

#### 2.2.1 定制Op算子

**在serving/op/目录下添加reader_op.cpp, classify_op.cpp, write_json_op.cpp**

- 预处理算子(ReaderOp, serving/op/reader_op.cpp)：从Request中读取图像字节流，通过opencv解码，填充tensor对象并输出到channel；
- 预测调用算子(ClassifyOp, serving/op/classify_op.cpp)：从ImageReaderOp的channel获得输入tensor，临时申请输出tensor，调用ModelToolkit进行预测，并将输出tensor写入channel
- 后处理算子(WriteJsonOp, serving/op/write_json.cpp)：从ImageClassifyop的channel获得输出tensor，将其序列化为json字符串，写入作为rpc的output

具体实现可参考demo中的源代码


#### 2.2.2 示例配置

关于Serving端的配置的详细信息，可以参考[Serving端配置](../Serving_Configure_CN.md)

以下配置文件将ReaderOP, ClassifyOP和WriteJsonOP串联成一个workflow (关于OP/workflow等概念，可参考[OP介绍](./OP_CN.md)和[DAG介绍](./DAG_CN.md))

- 配置文件示例：

**添加文件 serving/conf/service.prototxt** 

```shell![image](https://user-images.githubusercontent.com/16222477/141761999-e5b5016e-ca36-4479-82bf-a83fdb95f3c0.png)

services {
  name: "ImageClassifyService"
  workflows: "workflow1"
}
```

**添加文件 serving/conf/workflow.prototxt**

```shell
workflows {
  name: "workflow1"
  workflow_type: "Sequence"
  nodes {
    name: "image_reader_op"
    type: "ReaderOp"
  }
  nodes {
    name: "image_classify_op"
    type: "ClassifyOp"
    dependencies {
      name: "image_reader_op"
      mode: "RO"
    }
  }
  nodes {
    name: "write_json_op"
    type: "WriteJsonOp"
    dependencies {
      name: "image_classify_op"
      mode: "RO"
    }
  }
}
```

以下配置文件为模型加载配置

**添加文件 serving/conf/resource.prototxt**

```shell
model_manager_path: ./conf
model_manager_file: model_toolkit.prototxt
```

**添加文件 serving/conf/model_toolkit.prototxt**

```shell
engines {
  name: "image_classification_resnet"
  type: "FLUID_CPU_NATIVE_DIR"
  reloadable_meta: "./data/model/paddle/fluid_time_file"
  reloadable_type: "timestamp_ne"
  model_data_path: "./data/model/paddle/fluid/SE_ResNeXt50_32x4d"
  runtime_thread_num: 0
  batch_infer_size: 0
  enable_batch_align: 0
}
```


#### 2.2.3 代码编译

Serving端代码包含如下部分：
- protobuf接口文件，需要编译成.pb.cc及.pb.h文件并链接到最终可执行文件
- OP算子实现，需要链接到最终可执行文件
- Paddle serving框架代码，封装在libpdserving.a中，需要链接到最终可执行文件
- Paddle serving封装paddle-fluid预测库的代码，在inferencer-fluid-cpu/目录产出的libfluid_cpu_engine.a中
- 其他第三方依赖库：paddle预测库，brpc, opencv等

1) protobuf接口文件编译: 不能用protoc默认插件编译，需要编译成paddle serving定制的.pb.cc及.pb.h文件。具体命令是
```shell
$ protoc --cpp_out=/path/to/paddle-serving/build/serving/ --pdcodegen_out=/path/to/paddle-serving/ --plugin=protoc-gen-pdcodegen=/path/to/paddle-serving/build/predictor/pdcodegen --proto_path=/path/to/paddle-serving/predictor/proto
```
其中
`pdcodegen`是由predictor/src/pdcodegen.cpp编译成的protobuf编译插件, --proto_path用来指定去哪里寻找`import`语句需要的protobuf文件

predictor/proto目录下有serving端和client端都要包含的builtin_format.proto和pds_option.proto

**NOTE**
上述protoc命令在Paddle serving编译系统中被封装成一个CMake函数了，在cmake/generic.cmake::PROTOBUF_GENERATE_SERVING_CPP
CMakeLists.txt中调用函数的方法为：
```shell
PROTOBUF_GENERATE_SERVING_CPP(PROTO_SRCS PROTO_HDRS xxx.proto)
```

2) OP
serving/op/目录下OP对应的.cpp文件

3) Paddle Serving框架代码，封装在predictor目录产出的libpdserving.a中

4) Paddle Serving封装paddle-fluid预测库的代码，在inference-fluid-cpu/目录产出的libfluid_cpu_engine.a中

5) serving端main函数

为简化用户编写初始化代码的工作量，serving端必须的初始化过程已经由paddle Serving框架提供，请参考predictor/src/pdserving.cpp。该文件中包含了完整的初始化过程，用户只需提供合适的配置文件列表即可(请参考2.2.2节)，不必编写main函数

6) 第三方依赖库

brpc, paddle-fluid, opencv等第三方库，

7) 链接

整个链接过程在CMakeLists.txt中写法如下：
```shell
target_link_libraries(serving opencv_imgcodecs
        ${opencv_depend_libs} -Wl,--whole-archive fluid_cpu_engine
        -Wl,--no-whole-archive pdserving paddle_fluid ${paddle_depend_libs}
        ${MKLML_LIB} ${MKLML_IOMP_LIB} -lpthread -lcrypto -lm -lrt -lssl -ldl -lz)

```

### 2.3 gflags配置项

以下是serving端支持的gflag配置选项列表，并提供了默认值。

| name | 默认值 | 含义 |
|------|--------|------|
|workflow_path|./conf|workflow配置目录名|
|workflow_file|workflow.prototxt|workflow配置文件名|
|inferservice_path|./conf|service配置目录名|
|inferservice_file|service.prototxt|service配置文件名|
|resource_path|./conf|资源管理器目录名|
|resource_file|resource.prototxt|资源管理器文件名|
|reload_interval_s|10|重载线程间隔时间(s)|
|enable_model_toolkit|true|模型管理|
|enable_protocol_list|baidu_std|brpc 通信协议列表|
|log_dir|./log|log dir|
|num_threads|brpc server使用的系统线程数，默认为CPU核数|
|max_concurrency|并发处理的请求数，设为<=0则为不予限制，若大于0则限定brpc server端同时处理的请求数上限|

可以通过在serving/conf/gflags.conf覆盖默认值，例如
```
--log_dir=./serving_log/
```
将指定日志目录到./serving_log目录下


## 3. Client端

### 3.1 定义预测接口

**在sdk-cpp/proto添加image_class.proto**

与serving端预测接口protobuf文件基本一致，只要将`generate_impl=true`改为`generate_stub=true`

```c++
import "pds_option.proto";
...
service ImageClassifyService {
    rpc inference(Request) returns (Response);
    rpc debug(Request) returns (Response);
    // 改动：打开generate_stub开关(以支持配置驱动)
    option (pds.options).generate_stub = true;
};
```

### 3.2 Client端逻辑

Paddle Serving提供的C++ SDK在sdk-cpp/目录中，入口为sdk-cpp/include/predictor_sdk.h中的`class PredictorApi`类。

该类的主要接口：
```C++
class PredictroApi {
  // 创建PredictorApi句柄，输入为client端配置文件predictor.prototxt的目录和文件名
  int create(const char *path, const char *file);

  // 线程级初始化
  int thrd_initialize();

  // 根据名称获取Predictor句柄; ep_name对应predictor.prototxt中predictors的name字段
  Predictor *fetch_predictor(std::string ep_name);
};

class Predictor {
   // 预测
   int inference(google::protobuf::Message *req, google::protobuf::Message *res);

   // Debug模式
   int debug(google::protobuf::Message *req,
             google::protobuf::Message *res,
             buitl::IOBufBuilder *debug_os);
};
```

#### 3.2.1 请求逻辑

**增加sdk-cpp/demo/ximage.cpp**

```c++
// 进程级初始化
PredictorApi api;

if (api.create("./conf/", "predictors.prototxt") == 0) {
  return -1;
}

// 线程级预测调用：
Request req;
Response res;

api.thrd_initialize();

// Call this before every request
api.thrd_clear();

create_req(&req);

Predictor* predictor = api.fetch_predictor("ximage");
if (predictor == NULL) {
  return -1;
}

if (predictor->inference(req, res) != 0) {
  return -1;
}

// parse response
print_res(res);

// 线程级销毁
api.thrd_finalize();

// 进程级销毁
api.destroy();
```

具体实现可参考C++Serving提供的例子。sdk-cpp/demo/ximage.cpp

### 3.3 链接

Client端可执行文件包含的代码有：
- protobuf接口文件，需要编译成.pb.cc及.pb.h文件并链接到最终可执行文件
- main函数，以及调用SDK接口访问预测服务的逻辑，见3.2.1节
- Client端读取并维护predictor信息列表的代码，在sdk-cpp/目录产出的libsdk-cpp.a
- 因为protobuf接口文件用到了predictor/proto/目录下的builtin_format.proto和pds_option.proto，因此还需要联编libpdserving.a

1) protobuf接口文件，同serving端，需要用predictor/src/pdcodegen.cpp产出的pdcodegen插件，配合protoc使用，具体命令为
```shell
$ protoc --cpp_out=/path/to/paddle-serving/build/serving/ --pdcodegen_out=/path/to/paddle-serving/ --plugin=protoc-gen-pdcodegen=/path/to/paddle-serving/build/predictor/pdcodegen --proto_path=/path/to/paddle-serving/predictor/proto
```
其中
`pdcodegen`是由predictor/src/pdcodegen.cpp编译成的protobuf编译插件, --proto_path用来指定去哪里寻找`import`语句需要的protobuf文件

**NOTE**
上述protoc命令在Paddle Serving编译系统中被封装成一个CMake函数了，在cmake/generic.cmake::PROTOBUF_GENERATE_SERVING_CPP
CMakeLists.txt中调用函数的方法为：
```shell
PROTOBUF_GENERATE_SERVING_CPP(PROTO_SRCS PROTO_HDRS xxx.proto)
```
2) main函数，以及调用SDK接口访问预测服务的逻辑

3) Client端读取并维护predictor信息列表的代码，在sdk-cpp/目录产出的libsdk-cpp.a

4) predictor/目录产出的libpdserving.a

最终链接命令如下：

```shell
add_executable(ximage ${CMAKE_CURRENT_LIST_DIR}/demo/ximage.cpp)
target_link_libraries(ximage -Wl,--whole-archive sdk-cpp
               -Wl,--no-whole-archive pdserving -lpthread -lcrypto -lm -lrt -lssl -ldl
        -lz)

```

### 3.4 连接配置

**增加配置文件sdk/conf/predictors.prototxt**

```shell
## 默认配置共享
default_variant_conf {
  tag: "default"
  connection_conf {
    connect_timeout_ms: 2000
    rpc_timeout_ms: 20000
    connect_retry_count: 2
    max_connection_per_host: 100
    hedge_request_timeout_ms: -1
    hedge_fetch_retry_count: 2
    connection_type: "pooled"
  }
  naming_conf {
    cluster_filter_strategy: "Default"
    load_balance_strategy: "la"
  }
  rpc_parameter {
    compress_type: 0
    package_size: 20
    protocol: "baidu_std"
    max_channel_per_request: 3
  }
}
predictors {
  name: "ximage"
  service_name: "baidu.paddle_serving.predictor.image_classification.ImageClassifyService"
  endpoint_router: "WeightedRandomRender"
  weighted_random_render_conf {
    variant_weight_list: "50"
  }
  variants {
    tag: "var1"
    naming_conf {
      cluster: "list://127.0.0.1:8010"
    }
  }
}
```
关于客户端的详细配置选项，可参考[CLIENT CONFIGURATION](./Client_Configure_CN.md)
