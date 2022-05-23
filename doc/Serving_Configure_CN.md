# Serving配置

(简体中文|[English](./Serving_Configure_EN.md))

## 简介

本文主要介绍C++ Serving以及Python Pipeline的各项配置:

- [模型配置文件](#模型配置文件): 转换模型时自动生成，描述模型输入输出信息
- [C++ Serving](#c-serving): 用于高性能场景，介绍了快速启动以及自定义配置方法
- [Python Pipeline](#python-pipeline): 用于单算子多模型组合场景

## 模型配置文件

在开始介绍Server配置之前，先来介绍一下模型配置文件。我们在将模型转换为PaddleServing模型时，会生成对应的serving_client_conf.prototxt以及serving_server_conf.prototxt，两者内容一致，为模型输入输出的参数信息，方便用户拼装参数。该配置文件用于Server以及Client，并不需要用户自行修改。转换方法参考文档《[怎样保存用于Paddle Serving的模型](./Save_CN.md)》。protobuf格式可参考`core/configure/proto/general_model_config.proto`。

样例如下：

```
feed_var {
  name: "x"
  alias_name: "x"
  is_lod_tensor: false
  feed_type: 1
  shape: 13
}
fetch_var {
  name: "concat_1.tmp_0"
  alias_name: "concat_1.tmp_0"
  is_lod_tensor: false
  fetch_type: 1
  shape: 3
  shape: 640
  shape: 640
}
```

其中
- feed_var：模型输入
- fetch_var：模型输出
- name：名称
- alias_name：别名，与名称对应
- is_lod_tensor：是否为lod，具体可参考《[Lod字段说明](./LOD_CN.md)》
- feed_type：数据类型

|feed_type|类型|
|---------|----|
|0|INT64|
|1|FLOAT32|
|2|INT32|
|3|FP64|
|4|INT16|
|5|FP16|
|6|BF16|
|7|UINT8|
|8|INT8|
|20|STRING|

- shape：数据维度

## C++ Serving

### 1.快速启动与关闭

可以通过配置模型及端口号快速启动服务，启动命令如下：

```BASH
python3 -m paddle_serving_server.serve --model serving_model --port 9393
```

该命令会自动生成配置文件，并使用生成的配置文件启动C++ Serving。例如上述启动命令会自动生成workdir_9393目录，其结构如下

```
workdir_9393
├── general_infer_0
│   ├── fluid_time_file
│   ├── general_model.prototxt
│   └── model_toolkit.prototxt
├── infer_service.prototxt
├── resource.prototxt
└── workflow.prototxt
```

更多启动参数详见下表：
| Argument                                       | Type | Default | Description                                           |
| ---------------------------------------------- | ---- | ------- | ----------------------------------------------------- |
| `--thread`                                       | int  | `2`     | Number of brpc service thread                         |
| `--runtime_thread_num`                           | int[]| `0`     | Thread Number for each model in asynchronous mode     |
| `--batch_infer_size`                             | int[]| `32`    | Batch Number for each model in asynchronous mode      |
| `--gpu_ids`                                      | str[]| `"-1"`  | Gpu card id for each model                            |
| `--port`                                         | int  | `9292`  | Exposed port of current service to users              |
| `--model`                                        | str[]| `""`    | Path of paddle model directory to be served           |
| `--mem_optim_off`                                | -    | -       | Disable memory / graphic memory optimization          |
| `--ir_optim`                                     | bool | False   | Enable analysis and optimization of calculation graph |
| `--use_mkl` (Only for cpu version)               | -    | -       | Run inference with MKL. Need open with ir_optim.                                |
| `--use_trt` (Only for trt version)               | -    | -       | Run inference with TensorRT. Need open with ir_optim.                           |
| `--use_lite` (Only for Intel x86 CPU or ARM CPU) | -    | -       | Run PaddleLite inference. Need open with ir_optim.                              |
| `--use_xpu`                                      | -    | -       | Run PaddleLite inference with Baidu Kunlun XPU. Need open with ir_optim.        |
| `--precision`                                    | str  | FP32    | Precision Mode, support FP32, FP16, INT8              |
| `--use_calib`                                    | bool | False   | Use TRT int8 calibration                              |
| `--gpu_multi_stream`                             | bool | False   | EnableGpuMultiStream to get larger QPS                |
| `--use_ascend_cl`                                | bool | False   | Enable for ascend910; Use with use_lite for ascend310 |
| `--request_cache_size`                           | int  | `0`     | Bytes size of request cache. By default, the cache is disabled |
| `--enable_prometheus`                            | bool | False   | Use Prometheus |
| `--prometheus_port`                              | int  | 19393   | Port of the Prometheus |
| `--use_dist_model                                | bool | False   | Use distributed model or not |
| `--dist_carrier_id`                              | str  | ""      | Carrier id of distributed model |
| `--dist_cfg_file`                                | str  | ""      | Config file of distributed model |
| `--dist_endpoints`                               | str  | ""      | Endpoints of distributed model. splited by comma |
| `--dist_nranks`                                  | int  | 0       | The number of rank in the distributed model|
| `--dist_subgraph_index`                          | int  | -1      | The subgraph index of distributed model|
| `--dist_master_serving`                          | bool | False   | The master serving of distributed inference |
| `--min_subgraph_size`                            | str  | ""      | The min size of subgraph |
| `--gpu_memory_mb`                                | int  | 50      | Initially allocate GPU storage size, 50 MB default.|
| `--cpu_math_thread_num`                          | int  | 1       | Initialize the number of CPU computing threads|
| `--trt_workspace_size`                           | int  | 33554432| Initialize allocation 1 << 25 GPU storage size for tensorRT|
| `--trt_use_static`                               | bool | False   | Initialize TRT with static data| 

#### 当您的某个模型想使用多张GPU卡部署时.
```BASH
python3 -m paddle_serving_server.serve --model serving_model --thread 10 --port 9292 --gpu_ids 0,1,2
```
#### 当您的一个服务包含两个模型部署时.
```BASH
python3 -m paddle_serving_server.serve --model serving_model_1 serving_model_2 --thread 10 --port 9292
```
#### 当您想要关闭Serving服务时（在Serving启动目录或环境变量SERVING_HOME路径下，执行以下命令）.
```BASH
python3 -m paddle_serving_server.serve stop
```
stop参数发送SIGINT至C++ Serving，若改成kill则发送SIGKILL信号至C++ Serving

### 2.自定义配置启动

一般情况下，自动生成的配置可以应对大部分场景。对于特殊场景，用户也可自行定义配置文件。这些配置文件包括service.prototxt、workflow.prototxt、resource.prototxt、model_toolkit.prototxt、proj.conf。启动命令如下:
```BASH
/bin/serving --flagfile=proj.conf
```

#### 2.1 proj.conf

proj.conf用于传入服务参数，并指定了其他相关配置文件的路径。如果重复传入参数，则以最后序参数值为准。
```
# for paddle inference
--precision=fp32
--use_calib=False
--reload_interval_s=10
# for brpc
--max_concurrency=0
--num_threads=10
--bthread_concurrency=10
--max_body_size=536870912
# default path
--inferservice_path=conf
--inferservice_file=infer_service.prototxt
--resource_path=conf
--resource_file=resource.prototxt
--workflow_path=conf
--workflow_file=workflow.prototxt
```
各项参数的描述及默认值详见下表：
| name | Default | Description |
|------|--------|------|
|precision|"fp32"|Precision Mode, support FP32, FP16, INT8|
|use_calib|False|Only for deployment with TensorRT|
|reload_interval_s|10|Reload interval|
|max_concurrency|0|Limit of request processing in parallel, 0: unlimited|
|num_threads|10|Number of brpc service thread|
|bthread_concurrency|10|Number of bthread|
|max_body_size|536870912|Max size of brpc message|
|inferservice_path|"conf"|Path of inferservice conf|
|inferservice_file|"infer_service.prototxt"|Filename of inferservice conf|
|resource_path|"conf"|Path of resource conf|
|resource_file|"resource.prototxt"|Filename of resource conf|
|workflow_path|"conf"|Path of workflow conf|
|workflow_file|"workflow.prototxt"|Filename of workflow conf|

#### 2.2 service.prototxt

service.prototxt用于配置Paddle Serving实例挂载的service列表。通过`--inferservice_path`和`--inferservice_file`指定加载路径。protobuf格式可参考`core/configure/server_configure.protobuf`的`InferServiceConf`。示例如下：

```
port: 8010
services {
  name: "GeneralModelService"
  workflows: "workflow1"
}
```

其中：
- port: 用于配置Serving实例监听的端口号。
- services: 使用默认配置即可，不可修改。name指定service名称，workflow1的具体定义在workflow.prototxt

#### 2.3 workflow.prototxt

workflow.prototxt用来描述具体的workflow。通过`--workflow_path`和`--workflow_file`指定加载路径。protobuf格式可参考`configure/server_configure.protobuf`的`Workflow`类型。
如下示例，workflow由3个OP构成，GeneralReaderOp用于读取数据，GeneralInferOp依赖于GeneralReaderOp并进行预测，GeneralResponseOp将预测结果返回：

```
workflows {
  name: "workflow1"
  workflow_type: "Sequence"
  nodes {
    name: "general_reader_0"
    type: "GeneralReaderOp"
  }
  nodes {
    name: "general_infer_0"
    type: "GeneralInferOp"
    dependencies {
      name: "general_reader_0"
      mode: "RO"
    }
  }
  nodes {
    name: "general_response_0"
    type: "GeneralResponseOp"
    dependencies {
      name: "general_infer_0"
      mode: "RO"
    }
  }
}
```
其中：

- name: workflow名称，用于从service.prototxt索引到具体的workflow
- workflow_type: 只支持"Sequence"
- nodes: 用于串联成workflow的所有节点，可配置多个nodes。nodes间通过配置dependencies串联起来
- node.name: 与node.type一一对应，具体可参考`python/paddle_serving_server/dag.py`
- node.type: 当前node所执行OP的类名称，与serving/op/下每个具体的OP类的名称对应
- node.dependencies: 依赖的上游node列表
- node.dependencies.name: 与workflow内节点的name保持一致
- node.dependencies.mode: RO-Read Only, RW-Read Write

#### 2.4 resource.prototxt

resource.prototxt，用于指定模型配置文件。通过`--resource_path`和`--resource_file`指定加载路径。它的protobuf格式参考`core/configure/proto/server_configure.proto`的`ResourceConf`。示例如下：

```
model_toolkit_path: "conf"
model_toolkit_file: "general_infer_0/model_toolkit.prototxt"
general_model_path: "conf"
general_model_file: "general_infer_0/general_model.prototxt"
```

其中：

- model_toolkit_path:用来指定model_toolkit.prototxt所在的目录
- model_toolkit_file: 用来指定model_toolkit.prototxt所在的文件名
- general_model_path: 用来指定general_model.prototxt所在的目录
- general_model_file: 用来指定general_model.prototxt所在的文件名

#### 2.5 model_toolkit.prototxt

用来配置模型信息和预测引擎。它的protobuf格式参考`core/configure/proto/server_configure.proto`的ModelToolkitConf。model_toolkit.protobuf的磁盘路径不能通过命令行参数覆盖。示例如下：

```
engines {
  name: "general_infer_0"
  type: "PADDLE_INFER"
  reloadable_meta: "uci_housing_model/fluid_time_file"
  reloadable_type: "timestamp_ne"
  model_dir: "uci_housing_model"
  gpu_ids: -1
  enable_memory_optimization: true
  enable_ir_optimization: false
  use_trt: false
  use_lite: false
  use_xpu: false
  use_gpu: false
  combined_model: false
  gpu_multi_stream: false
  use_ascend_cl: false
  runtime_thread_num: 0
  batch_infer_size: 32
  enable_overrun: false
  allow_split_request: true
}
```

其中

- name: 引擎名称，与workflow.prototxt中的node.name以及所在目录名称对应
- type: 预测引擎的类型。当前只支持”PADDLE_INFER“
- reloadable_meta: 目前实际内容无意义，用来通过对该文件的mtime判断是否超过reload时间阈值
- reloadable_type: 检查reload条件：timestamp_ne/timestamp_gt/md5sum/revision/none

|reloadable_type|含义|
|---------------|----|
|timestamp_ne|reloadable_meta所指定文件的mtime时间戳发生变化|
|timestamp_gt|reloadable_meta所指定文件的mtime时间戳大于等于上次检查时记录的mtime时间戳|
|md5sum|目前无用，配置后永远不reload|
|revision|目前无用，配置后用于不reload|

- model_dir: 模型文件路径
- gpu_ids: 引擎运行时使用的GPU device id，支持指定多个，如：
```
# 指定GPU0，1，2
gpu_ids: 0
gpu_ids: 1
gpu_ids: 2
```
- enable_memory_optimization: 是否开启memory优化
- enable_ir_optimization: 是否开启ir优化
- use_trt: 是否开启TensorRT，需同时开启use_gpu
- use_lite: 是否开启PaddleLite
- use_xpu: 是否使用昆仑XPU
- use_gpu:是否使用GPU
- combined_model: 是否使用组合模型文件
- gpu_multi_stream: 是否开启gpu多流模式
- use_ascend_cl: 是否使用昇腾,单独开启适配昇腾910，同时开启lite适配310
- runtime_thread_num: 若大于0， 则启用Async异步模式，并创建对应数量的predictor实例。
- batch_infer_size: Async异步模式下的最大batch数
- enable_overrun: Async异步模式下总是将整个任务放入任务队列
- allow_split_request: Async异步模式下允许拆分任务

#### 2.6 general_model.prototxt

general_model.prototxt内容与模型配置serving_server_conf.prototxt相同，用了描述模型输入输出参数信息。示例如下：
```
feed_var {
  name: "x"
  alias_name: "x"
  is_lod_tensor: false
  feed_type: 1
  shape: 13
}
fetch_var {
  name: "fc_0.tmp_1"
  alias_name: "price"
  is_lod_tensor: false
  fetch_type: 1
  shape: 1
}
```

## Python Pipeline
### 快速启动与关闭
Python Pipeline启动命令如下：

```BASH
python3 web_service.py
```

当您想要关闭Serving服务时（在Pipeline启动目录下或环境变量SERVING_HOME路径下，执行以下命令）：
```BASH
python3 -m paddle_serving_server.serve stop
```
stop参数发送SIGINT至Pipeline Serving，若改成kill则发送SIGKILL信号至Pipeline Serving

### 配置文件
Python Pipeline提供了用户友好的多模型组合服务编程框架，适用于多模型组合应用的场景。
其配置文件为YAML格式，一般默认为config.yaml。示例如下：
```YAML
#rpc端口, rpc_port和http_port不允许同时为空。当rpc_port为空且http_port不为空时，会自动将rpc_port设置为http_port+1
rpc_port: 18090

#http端口, rpc_port和http_port不允许同时为空。当rpc_port可用且http_port为空时，不自动生成http_port
http_port: 9999

#worker_num, 最大并发数。当build_dag_each_worker=True时, 框架会创建worker_num个进程，每个进程内构建grpcSever和DAG
##当build_dag_each_worker=False时，框架会设置主线程grpc线程池的max_workers=worker_num
worker_num: 20

#build_dag_each_worker, False，框架在进程内创建一条DAG；True，框架会每个进程内创建多个独立的DAG
build_dag_each_worker: false

dag:
    #op资源类型, True, 为线程模型；False，为进程模型
    is_thread_op: False

    #重试次数
    retry: 1

    #使用性能分析, True，生成Timeline性能数据，对性能有一定影响；False为不使用
    use_profile: false
    tracer:
        interval_s: 10

    #client类型，包括brpc, grpc和local_predictor.local_predictor不启动Serving服务，进程内预测
    #client_type: local_predictor

    #channel的最大长度，默认为0
    #channel_size: 0

    #针对大模型分布式场景tensor并行，接收第一个返回结果后其他结果丢弃来提供速度
    #channel_recv_frist_arrive: False

op:
    det:
        #并发数，is_thread_op=True时，为线程并发；否则为进程并发
        concurrency: 6

        #Serving IPs
        #server_endpoints: ["127.0.0.1:9393"]

        #Fetch结果列表，以client_config中fetch_var的alias_name为准
        #fetch_list: ["concat_1.tmp_0"]

        #det模型client端配置
        #client_config: serving_client_conf.prototxt

        #Serving交互超时时间, 单位ms
        #timeout: 3000

        #Serving交互重试次数，默认不重试
        #retry: 1

        # 批量查询Serving的数量, 默认1。batch_size>1要设置auto_batching_timeout，否则不足batch_size时会阻塞
        #batch_size: 2

        # 批量查询超时，与batch_size配合使用
        #auto_batching_timeout: 2000

        #当op配置没有server_endpoints时，从local_service_conf读取本地服务配置
        local_service_conf:
            #client类型，包括brpc, grpc和local_predictor.local_predictor不启动Serving服务，进程内预测
            client_type: local_predictor

            #det模型路径
            model_config: ocr_det_model

            #Fetch结果列表，以client_config中fetch_var的alias_name为准
            fetch_list: ["concat_1.tmp_0"]

            # device_type, 0=cpu, 1=gpu, 2=tensorRT, 3=arm cpu, 4=kunlun xpu, 5=arm ascend310, 6=arm ascend910
            device_type: 0

            #计算硬件ID，当devices为""或不写时为CPU预测；当devices为"0", "0,1,2"时为GPU预测，表示使用的GPU卡
            devices: ""

            #use_mkldnn, 开启mkldnn时，必须同时设置ir_optim=True，否则无效
            #use_mkldnn: True

            #ir_optim, 开启TensorRT时，必须同时设置ir_optim=True，否则无效
            ir_optim: True
            
            #CPU 计算线程数，在CPU场景开启会降低单次请求响应时长
            #thread_num: 10
            
            #precsion, 预测精度，降低预测精度可提升预测速度
            #GPU 支持: "fp32"(default), "fp16", "int8"；
            #CPU 支持: "fp32"(default), "fp16", "bf16"(mkldnn); 不支持: "int8"
            precision: "fp32"

            #mem_optim, memory / graphic memory optimization
            #mem_optim: True

            #use_calib, Use TRT int8 calibration
            #use_calib: False

            #use_mkldnn, Use mkldnn for cpu
            #use_mkldnn: False

            #The cache capacity of different input shapes for mkldnn
            #mkldnn_cache_capacity: 0

            #mkldnn_op_list, op list accelerated using MKLDNN, None default
            #mkldnn_op_list: []

            #mkldnn_bf16_op_list,op list accelerated using MKLDNN bf16, None default.
            #mkldnn_bf16_op_list: []

            #min_subgraph_size,the minimal subgraph size for opening tensorrt to optimize, 3 default
            #min_subgraph_size: 3
    rec:
        #并发数，is_thread_op=True时，为线程并发；否则为进程并发
        concurrency: 3

        #超时时间, 单位ms
        timeout: -1

        #Serving交互重试次数，默认不重试
        retry: 1

        #当op配置没有server_endpoints时，从local_service_conf读取本地服务配置
        local_service_conf:

            #client类型，包括brpc, grpc和local_predictor。local_predictor不启动Serving服务，进程内预测
            client_type: local_predictor

            #rec模型路径
            model_config: ocr_rec_model

            #Fetch结果列表，以client_config中fetch_var的alias_name为准
            fetch_list: ["ctc_greedy_decoder_0.tmp_0", "softmax_0.tmp_0"]

            # device_type, 0=cpu, 1=gpu, 2=tensorRT, 3=arm cpu, 4=kunlun xpu, 5=arm ascend310, 6=arm ascend910
            device_type: 0

            #计算硬件ID，当devices为""或不写时为CPU预测；当devices为"0", "0,1,2"时为GPU预测，表示使用的GPU卡
            devices: ""

            #use_mkldnn, 开启mkldnn时，必须同时设置ir_optim=True，否则无效
            #use_mkldnn: True

            #ir_optim, 开启TensorRT时，必须同时设置ir_optim=True，否则无效
            ir_optim: True
            
            #CPU 计算线程数，在CPU场景开启会降低单次请求响应时长
            #thread_num: 10
            
            #precsion, 预测精度，降低预测精度可提升预测速度
            #GPU 支持: "fp32"(default), "fp16", "int8"；
            #CPU 支持: "fp32"(default), "fp16", "bf16"(mkldnn); 不支持: "int8"
            precision: "fp32"
```

### 单机多卡

单机多卡推理，M个OP进程与N个GPU卡绑定，需要在config.ymal中配置3个参数。首先选择进程模式，这样并发数即进程数，然后配置devices。绑定方法是进程启动时遍历GPU卡ID，例如启动7个OP进程，设置了0，1，2三个device id，那么第1、4、7个启动的进程与0卡绑定，第2、5进程与1卡绑定，3、6进程与卡2绑定。
```YAML
#op资源类型, True, 为线程模型；False，为进程模型
is_thread_op: False

#并发数，is_thread_op=True时，为线程并发；否则为进程并发
concurrency: 7

devices: "0,1,2"
```

### 异构硬件

Python Pipeline除了支持CPU、GPU之外，还支持多种异构硬件部署。在config.yaml中由device_type和devices控制。优先使用device_type指定，当其空缺时根据devices自动判断类型。device_type描述如下：
- CPU(Intel) : 0
- GPU : 1
- TensorRT : 2
- CPU(Arm) : 3
- XPU : 4
- Ascend310(Arm) : 5
- Ascend910(Arm) : 6

config.yml中硬件配置：
```YAML
#计算硬件类型: 空缺时由devices决定(CPU/GPU)，0=cpu, 1=gpu, 2=tensorRT, 3=arm cpu, 4=kunlun xpu, 5=arm ascend310, 6=arm ascend910
device_type: 0
#计算硬件ID，优先由device_type决定硬件类型。devices为""或空缺时为CPU预测；当为"0", "0,1,2"时为GPU预测，表示使用的GPU卡
devices: "" # "0,1"
```

### 低精度推理

Python Pipeline支持低精度推理，CPU、GPU和TensoRT支持的精度类型如下所示：
- CPU
  - fp32(default)
  - fp16
  - bf16(mkldnn)
- GPU
  - fp32(default)
  - fp16(TRT下有效)
  - int8
- Tensor RT
  - fp32(default)
  - fp16
  - int8 

```YAML
#precsion, 预测精度，降低预测精度可提升预测速度
#GPU 支持: "fp32"(default), "fp16(TensorRT)", "int8"；
#CPU 支持: "fp32"(default), "fp16", "bf16"(mkldnn); 不支持: "int8"
precision: "fp32"

#cablic, open it when using int8
use_calib: True
```
