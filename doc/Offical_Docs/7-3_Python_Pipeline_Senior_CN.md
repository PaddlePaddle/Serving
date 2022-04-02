# Python Pipeline 高阶用法

高阶用法在复杂场景中使用，实现更多自定义能力，包括 DAG 跳过某个OP运行、自定义数据传输结构以及多卡推理等。

## DAG 跳过某个OP运行

为 DAG 图中跳过某个 OP 运行，实际做法是在跳过此 OP 的 process 阶段，只要在 preprocess 做好判断，跳过 process 阶段，在和 postprocess 后直接返回即可。
preprocess 返回结果列表的第二个结果是 `is_skip_process=True` 表示是否跳过当前 OP 的 process 阶段，直接进入 postprocess 处理。

```python
def preprocess(self, input_dicts, data_id, log_id):
        """
        In preprocess stage, assembling data for process stage. users can 
        override this function for model feed features.
        Args:
            input_dicts: input data to be preprocessed
            data_id: inner unique id
            log_id: global unique id for RTT
        Return:
            input_dict: data for process stage
            is_skip_process: skip process stage or not, False default
            prod_errcode: None default, otherwise, product errores occured.
                          It is handled in the same way as exception. 
            prod_errinfo: "" default
        """
        # multiple previous Op
        if len(input_dicts) != 1:
            _LOGGER.critical(
                self._log(
                    "Failed to run preprocess: this Op has multiple previous "
                    "inputs. Please override this func."))
            os._exit(-1)
        (_, input_dict), = input_dicts.items()
        return input_dict, False, None, ""

```

## 自定义 proto 中 Request 和 Response 结构

当默认 proto 结构不满足业务需求时，同时下面2个文件的 proto 的 Request 和 Response message 结构，保持一致。

> pipeline/gateway/proto/gateway.proto 

> pipeline/proto/pipeline_service.proto

再重新编译 Serving Server。


## 自定义 URL
grpc gateway 处理 post 请求，默认 `method` 是 `prediction`，例如:127.0.0.1:8080/ocr/prediction。用户可自定义 name 和 method，对于已有 url 的服务可无缝切换。

```proto
service PipelineService {
  rpc inference(Request) returns (Response) {
    option (google.api.http) = {
      post : "/{name=*}/{method=*}"
      body : "*"
    };
  }
};
```

## 批量推理
Pipeline 支持批量推理，通过增大 batch size 可以提高 GPU 利用率。Python Pipeline 支持3种 batch 形式以及适用的场景如下：
- 场景1：一个推理请求包含批量数据(batch)
  - 单条数据定长，批量变长，数据转成BCHW格式
  - 单条数据变长，前处理中将单条数据做 padding 转成定长
- 场景2：一个推理请求的批量数据拆分成多个小块推理(mini-batch)
  - 由于 padding 会按最长对齐，当一批数据中有个"极大"尺寸数据时会导致推理变慢
  - 指定一个块大小，从而缩小"极大"尺寸数据的作用范围
- 场景3：合并多个请求数据批量推理(auto-batching)
  - 推理耗时明显长于前后处理，合并多个请求数据推理一次会提高吞吐和GPU利用率
  - 要求多个请求数据的 shape 一致

|                  接口                  |                    说明                     |
| :------------------------------------------: | :-----------------------------------------: |
|  batch | client 发送批量数据，client.predict 的 batch=True |
| mini-batch | preprocess 按 list 类型返回，参考 OCR 示例 RecOp的preprocess|
| auto-batching | config.yml 中 OP 级别设置 batch_size 和 auto_batching_timeout |


### 4.6 单机多卡
单机多卡推理，M 个 OP 进程与 N 个 GPU 卡绑定，在 `config.yml` 中配置3个参数有关系，首先选择进程模式、并发数即进程数，devices 是 GPU 卡 ID。绑定方法是进程启动时遍历 GPU 卡 ID，例如启动7个 OP 进程 `config.yml` 设置 devices:0,1,2，那么第1，4，7个启动的进程与0卡绑定，第2，4个启动的进程与1卡绑定，3，6进程与卡2绑定。
- 进程ID: 0  绑定 GPU 卡0
- 进程ID: 1  绑定 GPU 卡1
- 进程ID: 2  绑定 GPU 卡2
- 进程ID: 3  绑定 GPU 卡0
- 进程ID: 4  绑定 GPU 卡1
- 进程ID: 5  绑定 GPU 卡2
- 进程ID: 6  绑定 GPU 卡0

`config.yml` 中硬件配置：
```
#计算硬件 ID，当 devices 为""或不写时为 CPU 预测；当 devices 为"0", "0,1,2"时为 GPU 预测，表示使用的 GPU 卡
devices: "0,1,2"
```
