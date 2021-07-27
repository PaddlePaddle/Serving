# Serving Side Configuration


Paddle Serving配置文件格式采用明文格式的protobuf文件，配置文件的每个字段都需要事先在configure/proto/目录下相关.proto定义中定义好，才能被protobuf读取和解析到。

Serving端的所有配置均在configure/proto/server_configure.proto文件中。

## 1. service.prototxt
Serving端service 配置的入口是service.prototxt，用于配置Paddle Serving实例挂载的service列表。他的protobuf格式可参考`configure/server_configure.protobuf`的`InferServiceConf`类型。(至于具体的磁盘文件路径可通过--inferservice_path与--inferservice_file 命令行选项修改)，样例如下：

```JSON
port: 8010
services {
  name: "ImageClassifyService"
  workflows: "workflow1"
}
```

其中

- port: 该字段标明本机serving实例启动的监听端口。默认为8010。还可以通过--port=8010命令行参数指定。
- services: 可以配置多个services。Paddle Serving被设计为单个Serving实例可以同时承载多个预测服务，服务间通过service name进行区分。例如以下代码配置2个预测服务：
```JSON
port: 8010
services {
  name: "ImageClassifyService"
  workflows: "workflow1"
}
services {
  name: "BuiltinEchoService"
  workflows: "workflow2"
}
```

- service.name: 请填写serving/proto/xx.proto文件的service名称，例如，在serving/proto/image_class.proto中，service名称如下声明：
```JSON
service ImageClassifyService {
  rpc inference(Request) returns (Response);
  rpc debug(Request) returns (Response);
  option (pds.options).generate_impl = true;
};
```
则service name就是`ImageClassifyService`

- service.workflows: 用于指定该service下所配的workflow列表。可以配置多个workflow。在本例中，为`ImageClassifyService`配置了一个workflow：`workflow1`。`workflow1`的具体定义在workflow.prototxt

## 2. workflow.prototxt

workflow.prototxt用来描述每一个具体的workflow，他的protobuf格式可参考`configure/server_configure.protobuf`的`Workflow`类型。具体的磁盘文件路径可通过`--workflow_path`和`--workflow_file`指定。一个例子如下：

```JSON
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

workflows {
  name: "workflow2"
  workflow_type: "Sequence"
  nodes {
    name: "echo_op"
    type: "CommonEchoOp"
  }
}
```
以上样例配置了2个workflow：`workflow1`和`workflow2`。以`workflow1`为例：

- name: workflow名称，用于从service.prototxt索引到具体的workflow
- workflow_type: 可选"Sequence", "Parallel"，表示本workflow下节点所代表的OP是否可并行。**当前只支持Sequence类型，如配置了Parallel类型，则该workflow不会被执行**
- nodes: 用于串联成workflow的所有节点，可配置多个nodes。nodes间通过配置dependencies串联起来
- node.name: 随意，建议取一个能代表当前node所执行OP的类
- node.type: 当前node所执行OP的类名称，与serving/op/下每个具体的OP类的名称对应
- node.dependencies: 依赖的上游node列表
- node.dependencies.name: 与workflow内节点的name保持一致
- node.dependencies.mode: RO-Read Only, RW-Read Write

# 3. resource.prototxt

Serving端resource配置的入口是resource.prototxt，用于配置模型信息。它的protobuf格式参考`configure/proto/server_configure.proto`的ResourceConf。具体的磁盘文件路径可用`--resource_path`和`--resource_file`指定。样例如下：

```JSON
model_toolkit_path: "./conf"
model_toolkit_file: "model_toolkit.prototxt"
cube_config_file: "./conf/cube.conf"
```

其中：

- model_toolkit_path:用来指定model_toolkit.prototxt所在的目录
- model_toolkit_file: 用来指定model_toolkit.prototxt所在的文件名
- cube_config_file: 用来指定cube配置文件所在路径与文件名

Cube是Paddle Serving中用于大规模稀疏参数的组件。

# 4. model_toolkit.prototxt

用来配置模型信息和所用的预测引擎。它的protobuf格式参考`configure/proto/server_configure.proto`的ModelToolkitConf。model_toolkit.protobuf的磁盘路径不能通过命令行参数覆盖。样例如下：

```JSON
engines {
  name: "image_classification_resnet"
  type: "FLUID_CPU_NATIVE_DIR"
  reloadable_meta: "./data/model/paddle/fluid_time_file"
  reloadable_type: "timestamp_ne"
  model_data_path: "./data/model/paddle/fluid/SE_ResNeXt50_32x4d"
  runtime_thread_num: 0
  batch_infer_size: 0
  enable_batch_align: 0
  sparse_param_service_type: LOCAL
  sparse_param_service_table_name: "local_kv"
  enable_memory_optimization: true
  static_optimization: false
  force_update_static_cache: false
}
```

其中

- name: 模型名称。InferManager通过此名称，找到要使用的模型和预测引擎。可参考serving/op/classify_op.h与serving/op/classify_op.cpp的InferManager::instance().infer()方法的参数来了解。
- type: 预测引擎的类型。可在inferencer-fluid-cpu/src/fluid_cpu_engine.cpp找到当前注册的预测引擎列表

|预测引擎|含义|
|--------|----|
|FLUID_CPU_ANALYSIS|使用fluid Analysis API；模型所有参数保存在一个文件|
|FLUID_CPU_ANALYSIS_DIR|使用fluid Analysis API；模型所有参数分开保存为独立的文件，整个模型放到一个目录中|
|FLUID_CPU_NATIVE|使用fluid Native API；模型所有参数保存在一个文件|
|FLUID_CPU_NATIVE_DIR|使用fluid Native API；模型所有参数分开保存为独立的文件，整个模型放到一个目录中|
|FLUID_GPU_ANALYSIS|GPU预测，使用fluid Analysis API；模型所有参数保存在一个文件|
|FLUID_GPU_ANALYSIS_DIR|GPU预测，使用fluid Analysis API；模型所有参数分开保存为独立的文件，整个模型放到一个目录中|
|FLUID_GPU_NATIVE|GPU预测，使用fluid Native API；模型所有参数保存在一个文件|
|FLUID_GPU_NATIVE_DIR|GPU预测，使用fluid Native API；模型所有参数分开保存为独立的文件，整个模型放到一个目录中|


**fluid Analysis API和fluid Native API的区别**

Analysis API在模型加载过程中，会对模型计算逻辑进行多种优化，包括但不限于zero copy tensor，相邻OP的fuse等。**但优化逻辑不是一定对所有模型都有加速作用，有时甚至会有反作用，请以实测结果为准**。

- reloadable_meta: 目前实际内容无意义，用来通过对该文件的mtime判断是否超过reload时间阈值
- reloadable_type: 检查reload条件：timestamp_ne/timestamp_gt/md5sum/revision/none

|reloadable_type|含义|
|---------------|----|
|timestamp_ne|reloadable_meta所指定文件的mtime时间戳发生变化|
|timestamp_gt|reloadable_meta所指定文件的mtime时间戳大于等于上次检查时记录的mtime时间戳|
|md5sum|目前无用，配置后永远不reload|
|revision|目前无用，配置后用于不reload|

- model_data_path: 模型文件路径
- runtime_thread_num: 若大于0， 则启用bsf多线程调度框架，在每个预测bthread worker内启动多线程预测。要注意的是，当启用worker内多线程预测，workflow中OP需要用Serving框架的BatchTensor类做预测的输入和输出 (predictor/framework/infer_data.h, `class BatchTensor`)。
- batch_infer_size: 启用bsf多线程预测时，每个预测线程的batch size
- enable_batch_align:
- sparse_param_service_type: 枚举类型，可选参数，大规模稀疏参数服务类型

|sparse_param_service_type|含义|
|-------------------------|--|
|NONE|不使用大规模稀疏参数服务|
|LOCAL|单机本地大规模稀疏参数服务，以rocksdb作为引擎|
|REMOTE|分布式大规模稀疏参数服务，以Cube作为引擎|

- sparse_param_service_table_name: 可选参数，大规模稀疏参数服务承载本模型所用参数的表名。
- enable_memory_optimization: bool类型，可选参数，是否启用内存优化。只在使用fluid Analysis预测API时有意义。需要说明的是，在GPU预测时，会执行显存优化
- static_optimization: bool类型，是否执行静态优化。只有当启用内存优化时有意义。
- force_update_static_cache: bool类型，是否强制更新静态优化cache。只有当启用内存优化时有意义。

## 5. 命令行配置参数

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
|num_threads||brpc server使用的系统线程数，默认为CPU核数|
|port|8010|Serving进程接收请求监听端口|
|gpuid|0|GPU预测时指定Serving进程使用的GPU device id。只允许绑定1张GPU卡|
|bthread_concurrency|9|BRPC底层bthread的concurrency。在使用GPU预测引擎时，为了限制并发worker数，可使用此参数|
|bthread_min_concurrency|4|BRPC底层bthread的min concurrency。在使用GPU预测引擎时，为限制并发worker数，可使用此参数。与bthread_concurrency结合使用|

可以通过在serving/conf/gflags.conf覆盖默认值，例如
```
--log_dir=./serving_log/
```
将指定日志目录到./serving_log目录下

### 5.1 gflags.conf

可以将命令行配置参数写到配置文件中，该文件路径默认为`conf/gflags.conf`。如果`conf/gflags.conf`存在，则serving端会尝试解析其中的gflags命令。例如
```shell
--enable_model_toolkit
--port=8011
```

可用以下命令指定另外的命令行参数配置文件

```shell
bin/serving --g=true --flagfile=conf/gflags.conf.new
```
