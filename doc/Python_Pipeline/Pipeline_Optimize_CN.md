# Python Pipeline 优化指南

- [优化响应时长](#1)
  - [1.1 分析响应时长](#1.1)
    - [Pipeline Trace Tool](#1.1.1)
    - [Pipeline Profile Tool](#1.1.2)
  - [1.2 优化思路](#1.2)
- [优化服务吞吐](#2)
  - [2.1 分析吞吐瓶颈](#2.1)
  - [2.2 优化思路](#2.2)
    - [增加 Op 并发](#2.2.1)
    - [动态批量](#2.2.2)
    - [CPU 与 GPU 处理分离](#2.2.3)


通常，服务的性能优化是基于耗时分析，首先要掌握服务运行的各阶段耗时信息，从中找到耗时最长的性能瓶颈再做针对性优化。对于模型推理服务化不仅要关注耗时，由于 GPU 芯片昂贵，更要关注服务吞吐，从而提升 GPU 利用率实现降本增效。因此，模型推理服务化可总结为：
- 优化响应时长
- 优化服务吞吐

经过分析和调优后，各个阶段实现整体服务的性能最优。

<a name="1"></a>

## 优化响应时长

首先，优化响应时长的主要思路首先要掌握各阶段耗时，并分析出性能瓶颈或者耗时占比较高的阶段，再针对性能瓶颈做专项优化。

Paddle Serving 提供2种耗时分析工具，`Pipeline Trace Tool` 和 `Pipeline Profile Tool`。2个工具的特点如下：
- Pipeline Trace Tool : 统计服务端所有进程各个阶段的平均耗时，包括每个 `Op` 和 `Channel`，用于定量分析。
- Pipeline Profile Tool : 是可视化 Trace View 工具，生成多进程并发效果图，用定性和定量分析执行和并发效果。

<a name="1.1"></a>

**一.耗时分析**

<a name="1.1.1"></a>

1.Pipeline Trace Tool

`Pipeline Trace Tool` 统计每个 `Op` 和 `Channel` 中各阶段的处理耗时，

开启方法在配置文件 `config.yml` 的 `dag` 区段内添加 `tracer` 字段，框架会每隔 `interval_s` 时间生成 Trace 信息。
```
dag:
    #op资源类型, True, 为线程模型；False，为进程模型
    is_thread_op: True

    #tracer, 跟踪框架吞吐，每个OP和channel的工作情况。无tracer时不生成数据
    tracer:
        #每次trace的时间间隔，单位秒/s
        interval_s: 10
```

生成的 Trace 信息保存在 `./PipelineServingLogs/pipeline.tracer` 日志中。如下图所示
```
==================== TRACER ======================
 Op(uci):
         in[8473.507333333333 ms]：          # 等待前置 Channel 中数据放入 Op 的耗时，如长时间无请求，此值会变大
         prep[0.6753333333333333 ms]         # 推理前处理 preprocess 阶段耗时 
         midp[26.476333333333333 ms]         # 推理 process 阶段耗时
         postp[1.8616666666666666 ms]        # 推理后处理 postprocess 阶段耗时
         out[1.3236666666666668 ms]          # 后处理结果放入后置 channel 耗时
         idle[0.9965882097324374]            # 框架自循环耗时，间隔 1 ms，如此值很大说明系统负载高，调度变慢
 DAGExecutor:
         Query count[30]                     # interval_s 间隔时间内请求数量 
         QPS[27.35 q/s]                      # interval_s 间隔时间内服务 QPS 
         Succ[1.0]                           # interval_s 间隔时间内请求成功率 
         Error req[]                         # 异常请求信息
         Latency:                
                 ave[36.55233333333334 ms]   # 平均延时
                 .50[8.702 ms]               # 50分位延时
                 .60[8.702 ms]               # 60分位延时
                 .70[92.346 ms]              # 70分位延时
                 .80[92.346 ms]              # 70分位延时
                 .90[92.346 ms]              # 90分位延时
                 .95[92.346 ms]              # 95分位延时
                 .99[92.346 ms]              # 99分位延时
 Channel (server worker num[1]):
         chl0(In: ['@DAGExecutor'], Out: ['uci']) size[0/0]  # 框架 RequestOp 与 uci Op 之间 Channel 中堆积请求数。此值较大，说明下游 uci Op 消费能力不足。
         chl1(In: ['uci'], Out: ['@DAGExecutor']) size[0/0]  # uci Op 与 框架 ResponseOp 之间 Channel 中堆积的请求数。此值较大，说明下游 ReponseOp 消费能力不足。
 ==================== TRACER ======================
```
<a name="1.1.2"></a>

2.Pipeline Profile Tool

```
dag:
    #op资源类型, True, 为线程模型；False，为进程模型
    is_thread_op: True
    
    #使用性能分析, 默认为 False，imeline性能数据，对性能有一定影响
    use_profile: True,
```

开启后，Server 端在预测的过程中会将对应的日志信息打印到`标准输出`，为了更直观地展现各阶段的耗时，因此服务启动要使用如下命令：
```
python3.7 web_service.py > profile.txt 2>&1
```

服务接收请求后，输出 Profile 信息到 `profile.txt` 文件中。再粘贴如下代码到 `trace.py`， 使用框架提供 Analyst 模块对日志文件做进一步的分析处理。
```
from paddle_serving_server.pipeline import Analyst
import json
import sys

if __name__ == "__main__":
    log_filename = "profile.txt"
    trace_filename = "trace"
    analyst = Analyst(log_filename)
    analyst.save_trace(trace_filename)
```

运行命令，脚本将日志中的时间打点信息转换成 json 格式保存到 `trace` 文件。
```
python3.7 trace.py
```

`trace` 文件可以通过 `chrome` 浏览器的 `tracing` 功能进行可视化。
```
打开 chrome 浏览器，在地址栏输入 chrome://tracing/ ，跳转至 tracing 页面，点击 load 按钮，打开保存的 trace 文件，即可将预测服务的各阶段时间信息可视化。
```

通过图示中并发请求的处理流程可观测到推理阶段的流水线状态，以及多个请求在推理阶段的`间隔`信息，进行优化。

<a name="1.2"></a>

**二.降低响应时长优化思路**

根据 `Pipeline Trace Tool` 输出结果在不同阶段耗时长的问题，常见场景的优化方法如下：
- Op 推理阶段(midp) 耗时长:
  - 增加 Op 并发度
  - 开启 auto-batching (前提是多个请求的 shape 一致)
  - 若批量数据中某条数据的 shape 很大，padding 很大导致推理很慢，可参考 OCR 示例中 mini-batch 方法。
  - 开启 TensorRT/MKL-DNN 优化
  - 开启低精度推理
- Op 前处理阶段(prep) 或 后处理阶段耗时长:
  - 增加 OP 并发度
  - 优化前后处理逻辑
- in/out 耗时长（channel 堆积>5）
  - 检查 channel 传递的数据大小，可能为传输的数据大导致延迟大。
  - 优化传入数据，不传递数据或压缩后再传入
  - 增加 Op 并发度
  - 减少上游 Op 并发度

根据 `Pipeline Profile Tool` 输出结果优化流水行并发的效果
- 增加 Op 并发度，或调整不同 Op 的并发度
- 开启 auto-batching

此外，还有一些优化思路，如将 CPU 处理较慢的过程转换到 GPU 上处理等，客户端与服务端传输较大数据时，可使用共享内存方式传递内存或显存地址等。

<a name="2"></a>

## 优化服务吞吐

<a name="2.1"></a>

**一.分析吞吐瓶颈**

服务的吞吐量受到多种多因素条件制约，如 Op 处理时长、传输数据耗时、并发数和 DAG 图结构等，可以将这些因素进一步拆解，当传输数据不是极端庞大的时候，最重要因素是流水线中`最慢 Op 的处理时长和并发数`。
```
Op 处理时长：
op_cost = process(pre + mid + post) 

服务吞吐量：
service_throughput = 1 / 最慢 op_cost * 并发数

服务平响：
service_avg_cost = ∑op_concurrency 【关键路径】

批量预测平均耗时：
avg_batch_cost = (N * pre + mid + post) / N 
```
<a name="2.2"></a>

**二.优化思路**

优化吞吐的主要方法是 `增大 Op 并发数`、`自动批量` 和 `CPU 与 GPU 处理分离`

<a name="2.2.1"></a>

1.增加 Op 并发**

调整 Op 的并发数量通过设置 `is_thread_op: False` 进程类型 Op 和 `uci` Op 的 `concurrency` 字段
```
dag:
    #op资源类型, True, 为线程模型；False，为进程模型
    is_thread_op: False
op:
    uci:
        #并发数，is_thread_op=True时，为线程并发；否则为进程并发
        concurrency: 10
```
Op 的进程数量不是越大越好，受到机器 CPU 核数、内存和显存大小的限制，推荐设置 Op 的并发数不超过系统 CPU 核数。

<a name="2.2.2"></a>

2.动态批量

动态批量是增加吞吐的有一种方法，开启方式可参考[Python Pipeline 核心功能](./Pipeline_Features_CN.md#批量推理)

<a name="2.2.3"></a>

3.CPU 与 GPU 处理分离

在 `CV` 模型中，对图片或视频的前后处理成为主要瓶颈时，可考虑此方案，即将前后处理过程独立成一个 Op 并独立设置并发度。

将 CPU 前后处理和 GPU 推理过程比例调整到服务最佳配比。以 OCR 为例，原有流水线设计为 `RequestOp -> DetOp -> RecOp -> ResponseOp`。

根据耗时分析，`DetOp` 和 `RecOp` 的前处理耗时很长，因此，将2个模型前处理分离成独立 Op，最新的流水线设计为:

`RequestOp -> PreDetOp -> DetOp -> PreRecOp -> RecOp -> ResponseOp`，并调大 `PreDetOp` 和 `PreRecOp`的并发度，从而获得 20% 的性能提升。

由于增加了2次数据传递，单条请求的处理延时会增加。
