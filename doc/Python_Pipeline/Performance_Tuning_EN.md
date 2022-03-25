# Pipeline Serving Performance Optimization

(English|[简体中文](./Performance_Tuning_CN.md))


## 1. Performance analysis and optimization


### 1.1 How to optimize with the timeline tool

In order to better optimize the performance, PipelineServing provides a timeline tool to monitor the time of each stage of the whole service.

### 1.2 Output profile information on server side

The server is controlled by the `use_profile` field in yaml:

```yaml
dag:
    use_profile: true
```

After the function is enabled, the server will print the corresponding log information to the standard output in the process of prediction. In order to show the time consumption of each stage more intuitively, Analyst module is provided for further analysis and processing of log files.

The output of the server is first saved to a file. Taking `profile.txt` as an example, the script converts the time monitoring information in the log into JSON format and saves it to the `trace` file. The `trace` file can be visualized through the tracing function of Chrome browser.

```shell
from paddle_serving_server.pipeline import Analyst
import json
import sys

if __name__ == "__main__":
    log_filename = "profile.txt"
    trace_filename = "trace"
    analyst = Analyst(log_filename)
    analyst.save_trace(trace_filename)
```

Specific operation: open Chrome browser, input in the address bar `chrome://tracing/` , jump to the tracing page, click the load button, open the saved `trace` file, and then visualize the time information of each stage of the prediction service.

### 1.3 Output profile information on client side

The profile function can be enabled by setting `profile=True` in the `predict` interface on the client side.

After the function is enabled, the client will print the log information corresponding to the prediction to the standard output during the prediction process, and the subsequent analysis and processing are the same as that of the server.

### 1.4 Analytical methods
According to the time consumption of each stage in the pipeline.tracer log, the following formula is used to gradually analyze which stage is the main time consumption.

```
cost of one single OP：
op_cost = process(pre + mid + post) 

OP Concurrency: 
op_concurrency = op_cost(s) * qps_expected

Service throughput：
service_throughput = 1 / slowest_op_cost * op_concurrency

Service average cost：
service_avg_cost = ∑op_concurrency in critical Path

Channel accumulations：
channel_acc_size = QPS(down - up) * time

Average cost of batch predictor：
avg_batch_cost = (N * pre + mid + post) / N 
```

### 1.5 Optimization ideas
According to the long time consuming in stages below, different optimization methods are adopted. 
- OP Inference stage(mid-process):
  - Increase `concurrency`
  - Turn on `auto-batching`（Ensure that the shapes of multiple requests are consistent）
  - Use `mini-batch`, If the shape of data is very large.
  - Turn on TensorRT for GPU
  - Turn on MKLDNN for CPU
  - Turn on low precison inference
- OP preprocess or postprocess stage:
  - Increase `concurrency`
  - Optimize processing logic
- In/Out stage(channel accumulation > 5):
  - Check the size and delay of the data passed by the channel
  - Optimize the channel to transmit data, do not transmit data or compress it before passing it in
  - Increase `concurrency`
  - Decrease `concurrency` upstreams.
