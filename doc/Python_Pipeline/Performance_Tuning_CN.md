# Pipeline Serving 性能优化

([English](./Performance_Tuning_EN.md)|简体中文）

## 1. 性能分析与优化


### 1.1 如何通过 Timeline 工具进行优化

为了更好地对性能进行优化，PipelineServing 提供了 Timeline 工具，对整个服务的各个阶段时间进行打点。

### 1.2 在 Server 端输出 Profile 信息

Server 端用 yaml 中的 `use_profile` 字段进行控制：

```yaml
dag:
    use_profile: true
```

开启该功能后，Server 端在预测的过程中会将对应的日志信息打印到标准输出，为了更直观地展现各阶段的耗时，提供 Analyst 模块对日志文件做进一步的分析处理。

使用时先将 Server 的输出保存到文件，以 `profile.txt` 为例，脚本将日志中的时间打点信息转换成 json 格式保存到 `trace` 文件，`trace` 文件可以通过 chrome 浏览器的 tracing 功能进行可视化。

```python
from paddle_serving_server.pipeline import Analyst
import json
import sys

if __name__ == "__main__":
    log_filename = "profile.txt"
    trace_filename = "trace"
    analyst = Analyst(log_filename)
    analyst.save_trace(trace_filename)
```

具体操作：打开 chrome 浏览器，在地址栏输入 `chrome://tracing/` ，跳转至 tracing 页面，点击 load 按钮，打开保存的 `trace` 文件，即可将预测服务的各阶段时间信息可视化。

### 1.3 在 Client 端输出 Profile 信息

Client 端在 `predict` 接口设置 `profile=True`，即可开启 Profile 功能。

开启该功能后，Client 端在预测的过程中会将该次预测对应的日志信息打印到标准输出，后续分析处理同 Server。

### 1.4 分析方法
根据pipeline.tracer日志中的各个阶段耗时，按以下公式逐步分析出主要耗时在哪个阶段。
```
单OP耗时：
op_cost = process(pre + mid + post) 

OP期望并发数：
op_concurrency  = 单OP耗时(s) * 期望QPS

服务吞吐量：
service_throughput = 1 / 最慢OP的耗时 * 并发数

服务平响：
service_avg_cost = ∑op_concurrency 【关键路径】

Channel堆积：
channel_acc_size = QPS(down - up) * time

批量预测平均耗时：
avg_batch_cost = (N * pre + mid + post) / N 
```

### 1.5 优化思路
根据长耗时在不同阶段，采用不同的优化方法.
- OP推理阶段(mid-process):
  - 增加OP并发度
  - 开启auto-batching(前提是多个请求的shape一致)
  - 若批量数据中某条数据的shape很大，padding很大导致推理很慢，可使用mini-batch
  - 开启TensorRT/MKL-DNN优化
  - 开启低精度推理
- OP前处理阶段(pre-process):
  - 增加OP并发度
  - 优化前处理逻辑
- in/out耗时长（channel堆积>5）
  - 检查channel传递的数据大小和延迟
  - 优化传入数据，不传递数据或压缩后再传入
  - 增加OP并发度
  - 减少上游OP并发度
