# Python Pipeline 高阶用法

在复杂业务场景中使用常规功能无法满足需求，本文介绍一些高阶用法。
- DAG 结构跳过某个 Op 运行
- 批量推理
- 单机多卡推理
- 多种计算芯片上推理
- 低精度推理
- TensorRT 推理加速
- MKLDNN 推理加速


**一. DAG 结构跳过某个 Op 运行 **

此应用场景一般在 Op 前后处理中有 if 条件判断时，不满足条件时，跳过后面处理。实际做法是在跳过此 Op 的 process 阶段，只要在 preprocess 做好判断，跳过 process 阶段，在和 postprocess 后直接返回即可。
preprocess 返回结果列表的第二个结果是 `is_skip_process=True` 表示是否跳过当前 Op 的 process 阶段，直接进入 postprocess 处理。

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

** 二. 批量推理 **

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
| auto-batching | config.yml 中 Op 级别设置 batch_size 和 auto_batching_timeout |


** 三. 单机多卡推理 **

单机多卡推理，M 个 Op 进程与 N 个 GPU 卡绑定，在 `config.yml` 中配置3个参数有关系，首先选择进程模式、并发数即进程数，devices 是 GPU 卡 ID。绑定方法是进程启动时遍历 GPU 卡 ID，例如启动7个 Op 进程 `config.yml` 设置 devices:0,1,2，那么第1，4，7个启动的进程与0卡绑定，第2，4个启动的进程与1卡绑定，3，6进程与卡2绑定。
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


** 四. 多种计算芯片上推理 **

Pipeline 除了支持 CPU、GPU 芯片推理之外，还支持在多种计算硬件推理部署。在 `config.yml` 中由 `device_type` 和 `devices`。优先使用 `device_type` 指定类型，当空缺时根据 `devices` 判断。`device_type` 描述如下：
- CPU(Intel) : 0
- GPU(Jetson/海光DCU) : 1
- TensorRT : 2
- CPU(Arm) : 3
- XPU : 4
- Ascend310 : 5 
- ascend910 : 6

config.yml中硬件配置：
```
#计算硬件类型: 空缺时由devices决定(CPU/GPU)，0=cpu, 1=gpu, 2=tensorRT, 3=arm cpu, 4=kunlun xpu
device_type: 0

#计算硬件ID，优先由device_type决定硬件类型。devices为""或空缺时为CPU预测；当为"0", "0,1,2"时为GPU预测，表示使用的GPU卡
devices: "" # "0,1"
```
           
** 五. 低精度推理 **
Pipeline Serving支持低精度推理，CPU、GPU和TensoRT支持的精度类型如下图所示：

- CPU
  - fp32(default)
  - fp16
  - bf16(mkldnn)
- GPU
  - fp32(default)
  - fp16
  - int8
- Tensor RT
  - fp32(default)
  - fp16
  - int8 

使用int8时，要开启use_calib: True

参考[simple_web_service](../../examples/Pipeline/simple_web_service)示例
