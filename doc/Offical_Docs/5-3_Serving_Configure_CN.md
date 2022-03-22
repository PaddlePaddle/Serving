# Serving 配置

## 简介

本文主要介绍 C++ Serving 以及 Python Pipeline 的各项配置:

- [模型配置文件](#模型配置文件): 转换模型时自动生成，描述模型输入输出信息
- [C++ Serving](#c-serving): 用于高性能场景，介绍了快速启动以及自定义配置方法
- [Python Pipeline](#python-pipeline): 用于单算子多模型组合场景

## 模型配置文件

在开始介绍 Server 配置之前，先来介绍一下模型配置文件。我们在将模型转换为 PaddleServing 模型时，会生成对应的 serving_client_conf.prototxt 以及 serving_server_conf.prototxt，两者内容一致，为模型输入输出的参数信息，方便用户拼装参数。该配置文件用于 Server 以及 Client，并不需要用户自行修改。转换方法参考文档《[怎样保存用于Paddle Serving的模型](./Save_CN.md)》。protobuf 格式可参考 `core/configure/proto/general_model_config.proto`。

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
- is_lod_tensor：是否为 lod，具体可参考《[Lod字段说明](./LOD_CN.md)》
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

**一. 快速启动与关闭**

可以通过配置模型及端口号快速启动服务，启动命令如下：

```BASH
python3 -m paddle_serving_server.serve --model serving_model --port 9393
```

该命令会自动生成配置文件，并使用生成的配置文件启动 C++ Serving。例如上述启动命令会自动生成 workdir_9393 目录，其结构如下

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
| `thread`                                       | int  | `2`     | Number of brpc service thread                         |
| `runtime_thread_num`                           | int[]| `0`     | Thread Number for each model in asynchronous mode     |
| `batch_infer_size`                             | int[]| `32`    | Batch Number for each model in asynchronous mode      |
| `gpu_ids`                                      | str[]| `"-1"`  | Gpu card id for each model                            |
| `port`                                         | int  | `9292`  | Exposed port of current service to users              |
| `model`                                        | str[]| `""`    | Path of paddle model directory to be served           |
| `mem_optim_off`                                | -    | -       | Disable memory / graphic memory optimization          |
| `ir_optim`                                     | bool | False   | Enable analysis and optimization of calculation graph |
| `use_mkl` (Only for cpu version)               | -    | -       | Run inference with MKL. Need open with ir_optim.                                |
| `use_trt` (Only for trt version)               | -    | -       | Run inference with TensorRT. Need open with ir_optim.                           |
| `use_lite` (Only for Intel x86 CPU or ARM CPU) | -    | -       | Run PaddleLite inference. Need open with ir_optim.                              |
| `use_xpu`                                      | -    | -       | Run PaddleLite inference with Baidu Kunlun XPU. Need open with ir_optim.        |
| `precision`                                    | str  | FP32    | Precision Mode, support FP32, FP16, INT8              |
| `use_calib`                                    | bool | False   | Use TRT int8 calibration                              |
| `gpu_multi_stream`                             | bool | False   | EnableGpuMultiStream to get larger QPS                |
| `use_ascend_cl`                                | bool | False   | Enable for ascend910; Use with use_lite for ascend310 |
| `request_cache_size`                           | int  | `0`     | Bytes size of request cache. By default, the cache is disabled |

1. 当您的某个模型想使用多张 GPU 卡部署时.

```BASH
python3 -m paddle_serving_server.serve --model serving_model --thread 10 --port 9292 --gpu_ids 0,1,2
```

2. 当您的一个服务包含两个模型部署时.

```BASH
python3 -m paddle_serving_server.serve --model serving_model_1 serving_model_2 --thread 10 --port 9292
```

3. 当您想要关闭 Serving 服务时（在 Serving 启动目录或环境变量 SERVING_HOME 路径下，执行以下命令）.

```BASH
python3 -m paddle_serving_server.serve stop
```

stop 参数发送 SIGINT 至 C++ Serving，若改成 kill 则发送 SIGKILL 信号至 C++ Serving

**二. 自定义配置启动**

一般情况下，自动生成的配置可以应对大部分场景。对于特殊场景，用户也可自行定义配置文件。这些配置文件包括 service.prototxt、workflow.prototxt、resource.prototxt、model_toolkit.prototxt、proj.conf。启动命令如下:

```BASH
/bin/serving --flagfile=proj.conf
```

1. proj.conf

proj.conf 用于传入服务参数，并指定了其他相关配置文件的路径。如果重复传入参数，则以最后序参数值为准。
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

2. service.prototxt

service.prototxt 用于配置 Paddle Serving 实例挂载的 service 列表。通过 `--inferservice_path` 和 `--inferservice_file` 指定加载路径。protobuf 格式可参考 `core/configure/server_configure.protobuf` 的 `InferServiceConf`。示例如下：

```
port: 8010
services {
  name: "GeneralModelService"
  workflows: "workflow1"
}
```

其中：
- port: 用于配置 Serving 实例监听的端口号。
- services: 使用默认配置即可，不可修改。name 指定 service 名称，workflow1 的具体定义在 workflow.prototxt

3. workflow.prototxt

workflow.prototxt 用来描述具体的 workflow。通过 `--workflow_path` 和 `--workflow_file` 指定加载路径。protobuf 格式可参考 `configure/server_configure.protobuf` 的 `Workflow` 类型。
如下示例，workflow 由3个 OP 构成，GeneralReaderOp 用于读取数据，GeneralInferOp 依赖于 GeneralReaderOp 并进行预测，GeneralResponseOp 将预测结果返回：

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

- name: workflow 名称，用于从 service.prototxt 索引到具体的 workflow
- workflow_type: 只支持 "Sequence"
- nodes: 用于串联成 workflow 的所有节点，可配置多个 nodes。nodes 间通过配置 dependencies 串联起来
- node.name: 与 node.type 一一对应，具体可参考 `python/paddle_serving_server/dag.py`
- node.type: 当前 node 所执行 OP 的类名称，与 serving/op/ 下每个具体的 OP 类的名称对应
- node.dependencies: 依赖的上游 node 列表
- node.dependencies.name: 与 workflow 内节点的 name 保持一致
- node.dependencies.mode: RO-Read Only, RW-Read Write

4. resource.prototxt

resource.prototxt，用于指定模型配置文件。通过 `--resource_path` 和 `--resource_file` 指定加载路径。它的 protobuf 格式参考 `core/configure/proto/server_configure.proto` 的 `ResourceConf`。示例如下：

```
model_toolkit_path: "conf"
model_toolkit_file: "general_infer_0/model_toolkit.prototxt"
general_model_path: "conf"
general_model_file: "general_infer_0/general_model.prototxt"
```

其中：

- model_toolkit_path: 用来指定 model_toolkit.prototxt 所在的目录
- model_toolkit_file: 用来指定 model_toolkit.prototxt 所在的文件名
- general_model_path: 用来指定 general_model.prototxt 所在的目录
- general_model_file: 用来指定 general_model.prototxt 所在的文件名

5. model_toolkit.prototxt

用来配置模型信息和预测引擎。它的 protobuf 格式参考 `core/configure/proto/server_configure.proto` 的 ModelToolkitConf。model_toolkit.protobuf 的磁盘路径不能通过命令行参数覆盖。示例如下：

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

- name: 引擎名称，与 workflow.prototxt 中的 node.name 以及所在目录名称对应
- type: 预测引擎的类型。当前只支持 ”PADDLE_INFER“
- reloadable_meta: 目前实际内容无意义，用来通过对该文件的 mtime 判断是否超过 reload 时间阈值
- reloadable_type: 检查 reload 条件：timestamp_ne/timestamp_gt/md5sum/revision/none

|reloadable_type|含义|
|---------------|----|
|timestamp_ne|reloadable_meta 所指定文件的 mtime 时间戳发生变化|
|timestamp_gt|reloadable_meta 所指定文件的 mtime 时间戳大于等于上次检查时记录的 mtime 时间戳|
|md5sum|目前无用，配置后永远不 reload|
|revision|目前无用，配置后用于不 reload|

- model_dir: 模型文件路径
- gpu_ids: 引擎运行时使用的 GPU device id，支持指定多个，如：

```
# 指定 GPU 0，1，2
gpu_ids: 0
gpu_ids: 1
gpu_ids: 2
```

- enable_memory_optimization: 是否开启 memory 优化
- enable_ir_optimization: 是否开启 ir 优化
- use_trt: 是否开启 TensorRT，需同时开启 use_gpu
- use_lite: 是否开启 PaddleLite
- use_xpu: 是否使用昆仑 XPU
- use_gpu: 是否使用 GPU
- combined_model: 是否使用组合模型文件
- gpu_multi_stream: 是否开启 gpu 多流模式
- use_ascend_cl: 是否使用昇腾,单独开启适配昇腾 910，同时开启 lite 适配 310
- runtime_thread_num: 若大于 0， 则启用 Async 异步模式，并创建对应数量的 predictor 实例。
- batch_infer_size: Async 异步模式下的最大 batch 数
- enable_overrun: Async 异步模式下总是将整个任务放入任务队列
- allow_split_request: Async 异步模式下允许拆分任务

6. general_model.prototxt

general_model.prototxt 内容与模型配置 serving_server_conf.prototxt 相同，用了描述模型输入输出参数信息。示例如下：
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

**一. 快速启动与关闭**

Python Pipeline 启动命令如下：

```BASH
python3 web_service.py
```

当您想要关闭 Serving 服务时（在 Pipeline 启动目录下或环境变量 SERVING_HOME 路径下，执行以下命令）：

```BASH
python3 -m paddle_serving_server.serve stop
```

stop 参数发送 SIGINT 至 Pipeline Serving，若改成 kill 则发送 SIGKILL 信号至 Pipeline Serving

**二. 配置文件**

Python Pipeline 提供了用户友好的多模型组合服务编程框架，适用于多模型组合应用的场景。
其配置文件为 YAML 格式，一般默认为 config.yaml。示例如下：

```YAML
#rpc 端口, rpc_port 和 http_port 不允许同时为空。当 rpc_port 为空且 http_port 不为空时，会自动将 rpc_port 设置为 http_port+1
rpc_port: 18090

#http 端口, rpc_port 和 http_port 不允许同时为空。当 rpc_port 可用且 http_port 为空时，不自动生成 http_port
http_port: 9999

#worker_num, 最大并发数。当 build_dag_each_worker=True 时, 框架会创建w orker_num 个进程，每个进程内构建 grpcSever和DAG
##当 build_dag_each_worker=False 时，框架会设置主线程 grpc 线程池的 max_workers=worker_num
worker_num: 20

#build_dag_each_worker, False，框架在进程内创建一条 DAG；True，框架会每个进程内创建多个独立的 DAG
build_dag_each_worker: false

dag:
    #op 资源类型, True, 为线程模型；False，为进程模型
    is_thread_op: False

    #重试次数
    retry: 1

    #使用性能分析, True，生成 Timeline 性能数据，对性能有一定影响；False 为不使用
    use_profile: false
    tracer:
        interval_s: 10

    #client 类型，包括 brpc, grpc 和 local_predictor.local_predictor 不启动 Serving 服务，进程内预测
    #client_type: local_predictor

    #channel 的最大长度，默认为0
    #channel_size: 0

    #针对大模型分布式场景 tensor 并行，接收第一个返回结果后其他结果丢弃来提供速度
    #channel_recv_frist_arrive: False

op:
    det:
        #并发数，is_thread_op=True 时，为线程并发；否则为进程并发
        concurrency: 6

        #Serving IPs
        #server_endpoints: ["127.0.0.1:9393"]

        #Fetch 结果列表，以 client_config 中 fetch_var 的 alias_name 为准
        #fetch_list: ["concat_1.tmp_0"]

        #det 模型 client 端配置
        #client_config: serving_client_conf.prototxt

        #Serving 交互超时时间, 单位 ms
        #timeout: 3000

        #Serving 交互重试次数，默认不重试
        #retry: 1

        # 批量查询 Serving 的数量, 默认 1。batch_size>1 要设置 auto_batching_timeout，否则不足 batch_size 时会阻塞
        #batch_size: 2

        # 批量查询超时，与 batch_size 配合使用
        #auto_batching_timeout: 2000

        #当 op 配置没有 server_endpoints 时，从 local_service_conf 读取本地服务配置
        local_service_conf:
            #client 类型，包括 brpc, grpc 和 local_predictor.local_predictor 不启动 Serving 服务，进程内预测
            client_type: local_predictor

            #det 模型路径
            model_config: ocr_det_model

            #Fetch 结果列表，以 client_config 中 fetch_var 的 alias_name 为准
            fetch_list: ["concat_1.tmp_0"]

            # device_type, 0=cpu, 1=gpu, 2=tensorRT, 3=arm cpu, 4=kunlun xpu, 5=arm ascend310, 6=arm ascend910
            device_type: 0

            #计算硬件 ID，当 devices 为""或不写时为 CPU 预测；当 devices 为 "0", "0,1,2" 时为 GPU 预测，表示使用的 GPU 卡
            devices: ""

            #use_mkldnn, 开启 mkldnn 时，必须同时设置 ir_optim=True，否则无效
            #use_mkldnn: True

            #ir_optim, 开启 TensorRT 时，必须同时设置 ir_optim=True，否则无效
            ir_optim: True
            
            #CPU 计算线程数，在 CPU 场景开启会降低单次请求响应时长
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
        #并发数，is_thread_op=True 时，为线程并发；否则为进程并发
        concurrency: 3

        #超时时间, 单位 ms
        timeout: -1

        #Serving 交互重试次数，默认不重试
        retry: 1

        #当 op 配置没有 server_endpoints 时，从 local_service_conf 读取本地服务配置
        local_service_conf:

            #client 类型，包括 brpc, grpc 和 local_predictor。local_predictor 不启动 Serving 服务，进程内预测
            client_type: local_predictor

            #rec 模型路径
            model_config: ocr_rec_model

            #Fetch 结果列表，以 client_config 中 fetch_var 的 alias_name 为准
            fetch_list: ["ctc_greedy_decoder_0.tmp_0", "softmax_0.tmp_0"]

            # device_type, 0=cpu, 1=gpu, 2=tensorRT, 3=arm cpu, 4=kunlun xpu, 5=arm ascend310, 6=arm ascend910
            device_type: 0

            #计算硬件 ID，当 devices 为""或不写时为 CPU 预测；当 devices 为 "0", "0,1,2" 时为 GPU 预测，表示使用的 GPU 卡
            devices: ""

            #use_mkldnn, 开启 mkldnn 时，必须同时设置 ir_optim=True，否则无效
            #use_mkldnn: True

            #ir_optim, 开启 TensorRT 时，必须同时设置 ir_optim=True，否则无效
            ir_optim: True
            
            #CPU 计算线程数，在 CPU 场景开启会降低单次请求响应时长
            #thread_num: 10
            
            #precsion, 预测精度，降低预测精度可提升预测速度
            #GPU 支持: "fp32"(default), "fp16", "int8"；
            #CPU 支持: "fp32"(default), "fp16", "bf16"(mkldnn); 不支持: "int8"
            precision: "fp32"
```

**三. 单机多卡**

单机多卡推理，M 个 OP 进程与 N 个 GPU 卡绑定，需要在 config.ymal 中配置 3 个参数。首先选择进程模式，这样并发数即进程数，然后配置 devices。绑定方法是进程启动时遍历 GPU 卡 ID，例如启动 7 个 OP 进程，设置了 0，1，2 三个 device id，那么第 1、4、7 个启动的进程与 0 卡绑定，第 2、5 进程与 1 卡绑定，3、6 进程与卡 2 绑定。

```YAML
#op 资源类型, True, 为线程模型；False，为进程模型
is_thread_op: False

#并发数，is_thread_op=True 时，为线程并发；否则为进程并发
concurrency: 7

devices: "0,1,2"
```

**四. 异构硬件**

Python Pipeline 除了支持 CPU、GPU 之外，还支持多种异构硬件部署。在 config.yaml 中由 device_type 和 devices 控制。优先使用 device_type 指定，当其空缺时根据 devices 自动判断类型。device_type 描述如下：

- CPU(Intel) : 0
- GPU : 1
- TensorRT : 2
- CPU(Arm) : 3
- XPU : 4
- Ascend310(Arm) : 5
- Ascend910(Arm) : 6

config.yml 中硬件配置：

```YAML
#计算硬件类型: 空缺时由 devices 决定( CPU/GPU )，0=cpu, 1=gpu, 2=tensorRT, 3=arm cpu, 4=kunlun xpu, 5=arm ascend310, 6=arm ascend910
device_type: 0
#计算硬件 ID，优先由 device_type 决定硬件类型。devices 为""或空缺时为 CPU 预测；当为 "0", "0,1,2" 时为 GPU 预测，表示使用的 GPU 卡
devices: "" # "0,1"
```

**五. 低精度推理**

Python Pipeline 支持低精度推理，CPU、GPU 和 TensoRT 支持的精度类型如下所示：
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
