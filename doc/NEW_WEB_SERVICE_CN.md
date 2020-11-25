# 如何开发一个新的Web Service？


(简体中文|[English](NEW_WEB_SERVICE.md))

本文档将以 Uci 房价预测服务为例，来介绍如何开发一个新的Web Service。您可以在[这里](../python/examples/pipeline/simple_web_service/web_service.py)查阅完整的代码。

## Op 基类

在一些服务中，单个模型可能无法满足需求，需要多个模型串联或并联来完成整个服务。我们将单个模型操作称为 Op，并提供了一套简单的接口来实现 Op 串联或并联的复杂逻辑。

Op 间数据是以字典形式进行传递的，Op 可以以线程或进程方式启动，同时可以对 Op 的并发数等进行配置。

通常情况下，您需要继承 Op 基类，重写它的 `init_op`、`preprocess` 和 `postprocess` 方法，默认实现如下：

```python
class Op(object):
  def init_op(self):
    pass
  def preprocess(self, input_dicts):
    # multiple previous Op
    if len(input_dicts) != 1:
      _LOGGER.critical(
        "Failed to run preprocess: this Op has multiple previous "
        "inputs. Please override this func.")
      os._exit(-1)
    (_, input_dict), = input_dicts.items()
    return input_dict
  def postprocess(self, input_dicts, fetch_dict):
    return fetch_dict
```

### init_op 方法

该方法用于加载用户自定义资源（如字典等），在 [UciOp](../python/examples/pipeline/simple_web_service/web_service.py) 中加载了一个分隔符。

**注意**：如果 Op 是以线程模式加载的，那么在 Op 多并发时，同种 Op 的不同线程只执行一次 `init_op`，且共用 `init_op` 加载的资源。

### preprocess 方法

该方法用于模型预测前对数据的预处理，它有一个 `input_dicts` 参数，`input_dicts` 是一个字典，key 为前继 Op 的 `name`，value 为对应前继 Op 传递过来的数据（数据同样是字典格式）。

`preprocess` 方法需要将数据处理成 ndarray 字典（key 为 feed 变量名，value 为对应的 ndarray 值），Op 会将该返回值作为模型预测的输入，并将输出传递给 `postprocess` 方法。

**注意**：如果 Op 没有配置模型，则 `preprocess` 的返回值会直接传递给 `postprocess`。

### postprocess 方法

该方法用于模型预测后对数据的后处理，它有两个参数，`input_dicts` 和 `fetch_dict`。

其中，`input_dicts` 与 `preprocess` 的参数相同，`fetch_dict` 为模型预测的输出（key 为 fetch 变量名，value 为对应的 ndarray 值）。Op 会将 `postprocess` 的返回值作为后继 Op `preprocess` 的输入。

**注意**：如果 Op 没有配置模型，则 `fetch_dict` 将为 `preprocess` 的返回值。



下面是 Uci 例子的 Op：

```python
class UciOp(Op):
    def init_op(self):
        self.separator = ","

    def preprocess(self, input_dicts):
        (_, input_dict), = input_dicts.items()
        x_value = input_dict["x"]
        if isinstance(x_value, (str, unicode)):
            input_dict["x"] = np.array(
                [float(x.strip()) for x in x_value.split(self.separator)])
        return input_dict

    def postprocess(self, input_dicts, fetch_dict):
        fetch_dict["price"] = str(fetch_dict["price"][0][0])
        return fetch_dict
```



## WebService 基类

Paddle Serving 实现了 [WebService](https://github.com/PaddlePaddle/Serving/blob/develop/python/paddle_serving_server/web_service.py#L28) 基类，您需要重写它的 `get_pipeline_response` 方法来定义 Op 间的拓扑关系，并返回作为 Response 的 Op，默认实现如下：

```python
class WebService(object):
  def get_pipeline_response(self, read_op):
    return None
```

其中，`read_op` 作为整个服务拓扑图的入口（即用户自定义的第一个 Op 的前继为 `read_op`）。

对于单 Op 服务（单模型），以 Uci 服务为例（整个服务中只有一个 Uci 房价预测模型）：

```python
class UciService(WebService):
  def get_pipeline_response(self, read_op):
    uci_op = UciOp(name="uci", input_ops=[read_op])
    return uci_op
```

对于多 Op 服务（多模型），以 Ocr 服务为例（整个服务由 Det 模型和 Rec 模型串联完成）：

```python
class OcrService(WebService):
  def get_pipeline_response(self, read_op):
    det_op = DetOp(name="det", input_ops=[read_op])
    rec_op = RecOp(name="rec", input_ops=[det_op])
    return rec_op
```



WebService 对象需要通过 `prepare_pipeline_config` 加载一个 yaml 配置文件，用来对各个 Op 以及整个服务进行配置，最简单的配置文件如下（Uci 例子）：

```yaml
http_port: 18080
op:
    uci:
        local_service_conf:
            model_config: uci_housing_model # 路径
```

yaml 文件的所有字段名详见下面：

```yaml
rpc_port: 18080  # gRPC端口号
build_dag_each_worker: false  # 是否使用进程版 Servicer，默认为 false
worker_num: 1  # gRPC线程池大小（进程版 Servicer 中为进程数），默认为 1
http_port: 0 # HTTP 服务的端口号，若该值小于或等于 0 则不开启 HTTP 服务，默认为 0
dag:
    is_thread_op: true  # 是否使用线程版Op，默认为 true
    client_type: brpc  # 使用 brpc 或 grpc client，默认为 brpc
    retry: 1  # DAG Executor 在失败后重试次数，默认为 1，即不重试
    use_profile: false  # 是否在 Server 端打印日志，默认为 false
    tracer:
        interval_s: -1 # Tracer 监控的时间间隔，单位为秒。当该值小于 1 时不启动监控，默认为 -1
op:
    <op_name>: # op 名，与程序中定义的相对应
        concurrency: 1 # op 并发数，默认为 1
        timeout: -1 # 预测超时时间，单位为毫秒。默认为 -1 即不超时
        retry: 1 # 超时重发次数。默认为 1 即不重试
        batch_size: 1 # auto-batching 中的 batch_size，若设置该字段则 Op 会将多个请求输出合并为一个 batch
        auto_batching_timeout: -1 # auto-batching 超时时间，单位为毫秒。默认为 -1 即不超时
        local_service_conf:
            model_config: # 对应模型文件的路径，无默认值（None）。若不配置该项则不会加载模型文件。
            workdir: "" # 对应模型的工作目录
            thread_num: 2 # 对应模型用几个线程启动
            devices: "" # 模型启动在哪个设备上，可以指定 gpu 卡号（如 "0,1,2"），默认为 cpu
            mem_optim: true # mem 优化选项，默认为 true
            ir_optim: false # ir 优化选项，默认为 false
```

其中，Op 的所有字段均可以在程序中创建 Op 时定义（会覆盖 yaml 的字段）。
