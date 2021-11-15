## Paddle Serving 快速开始示例

([English](./Quick_Start_EN.md)|简体中文)

这个快速开始示例主要是为了给那些已经有一个要部署的模型的用户准备的，而且我们也提供了一个可以用来部署的模型。如果您想知道如何从离线训练到在线服务走完全流程，请参考前文的AiStudio教程。

<h3 align="center">波士顿房价预测</h3>

进入到Serving的git目录下，进入到`fit_a_line`例子
``` shell
cd Serving/examples/C++//fit_a_line
sh get_data.sh
```

Paddle Serving 为用户提供了基于 HTTP 和 RPC 的服务



<h3 align="center">RPC服务</h3>

用户还可以使用`paddle_serving_server.serve`启动RPC服务。 尽管用户需要基于Paddle Serving的python客户端API进行一些开发，但是RPC服务通常比HTTP服务更快。需要指出的是这里我们没有指定`--name`。

``` shell
python3 -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9292
```
<center>

| Argument                                       | Type | Default | Description                                           |
| ---------------------------------------------- | ---- | ------- | ----------------------------------------------------- |
| `thread`                                       | int  | `2`     | Number of brpc service thread                         |
| `op_num`                                       | int[]| `0`     | Thread Number for each model in asynchronous mode     |
| `op_max_batch`                                 | int[]| `32`    | Batch Number for each model in asynchronous mode      |
| `gpu_ids`                                      | str[]| `"-1"`  | Gpu card id for each model                            |
| `port`                                         | int  | `9292`  | Exposed port of current service to users              |
| `model`                                        | str[]| `""`    | Path of paddle model directory to be served           |
| `mem_optim_off`                                | -    | -       | Disable memory / graphic memory optimization          |
| `ir_optim`                                     | bool | False   | Enable analysis and optimization of calculation graph |
| `use_mkl` (Only for cpu version)               | -    | -       | Run inference with MKL                                |
| `use_trt` (Only for trt version)               | -    | -       | Run inference with TensorRT                           |
| `use_lite` (Only for Intel x86 CPU or ARM CPU) | -    | -       | Run PaddleLite inference                              |
| `use_xpu`                                      | -    | -       | Run PaddleLite inference with Baidu Kunlun XPU        |
| `precision`                                    | str  | FP32    | Precision Mode, support FP32, FP16, INT8              |
| `use_calib`                                    | bool | False   | Use TRT int8 calibration                              |
| `gpu_multi_stream`                             | bool | False   | EnableGpuMultiStream to get larger QPS                |

#### 异步模型的说明
    异步模式适用于1、请求数量非常大的情况，2、多模型串联，想要分别指定每个模型的并发数的情况。
    异步模式有助于提高Service服务的吞吐（QPS），但对于单次请求而言，时延会有少量增加。
    异步模式中，每个模型会启动您指定个数的N个线程，每个线程中包含一个模型实例，换句话说每个模型相当于包含N个线程的线程池，从线程池的任务队列中取任务来执行。
    异步模式中，各个RPC Server的线程只负责将Request请求放入模型线程池的任务队列中，等任务被执行完毕后，再从任务队列中取出已完成的任务。
    上表中通过 --thread 10 指定的是RPC Server的线程数量，默认值为2，--op_num 指定的是各个模型的线程池中线程数N，默认值为0，表示不使用异步模式。
    --op_max_batch 指定的各个模型的batch数量，默认值为32，该参数只有当--op_num不为0时才生效。
    
#### 当您的某个模型想使用多张GPU卡部署时.
python3 -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9292 --gpu_ids 0,1,2
#### 当您的一个服务包含两个模型部署时.
python3 -m paddle_serving_server.serve --model uci_housing_model_1 uci_housing_model_2 --thread 10 --port 9292
#### 当您的一个服务包含两个模型，且每个模型都需要指定多张GPU卡部署时.
python3 -m paddle_serving_server.serve --model uci_housing_model_1 uci_housing_model_2 --thread 10 --port 9292 --gpu_ids 0,1 1,2
#### 当您的一个服务包含两个模型，且每个模型都需要指定多张GPU卡，且需要异步模式每个模型指定不同的并发数时.
python3 -m paddle_serving_server.serve --model uci_housing_model_1 uci_housing_model_2 --thread 10 --port 9292 --gpu_ids 0,1 1,2 --op_num 4 8



</center>

``` python
# A user can visit rpc service through paddle_serving_client API
from paddle_serving_client import Client

client = Client()
client.load_client_config("uci_housing_client/serving_client_conf.prototxt")
client.connect(["127.0.0.1:9292"])
data = [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727,
        -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]
fetch_map = client.predict(feed={"x": np.array(data).reshape(1,13,1)}, fetch=["price"])
print(fetch_map)

```
在这里，`client.predict`函数具有两个参数。 `feed`是带有模型输入变量别名和值的`python dict`。 `fetch`被要从服务器返回的预测变量赋值。 在该示例中，在训练过程中保存可服务模型时，被赋值的tensor名为`"x"`和`"price"`。


<h3 align="center">HTTP服务</h3>

用户也可以将数据格式处理逻辑放在服务器端进行，这样就可以直接用curl去访问服务，参考如下案例，在目录`python/examples/fit_a_line`.

```
python3 -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9292 --name uci
```
客户端输入
```
curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"x": [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]}], "fetch":["price"]}' http://127.0.0.1:9292/uci/prediction
```
返回结果
```
{"result":{"price":[[18.901151657104492]]}}
```

<h3 align="center">Pipeline服务</h3>

Paddle Serving提供业界领先的多模型串联服务，强力支持各大公司实际运行的业务场景，参考 [OCR文字识别案例](../examples/Pipeline/PaddleOCR/ocr/)，在目录`python/examples/pipeline/ocr`

我们先获取两个模型
```
python3 -m paddle_serving_app.package --get_model ocr_rec
tar -xzvf ocr_rec.tar.gz
python3 -m paddle_serving_app.package --get_model ocr_det
tar -xzvf ocr_det.tar.gz
```
然后启动服务端程序，将两个串联的模型作为一个整体的服务。
```
python3 web_service.py
```
最终使用http的方式请求
```
python3 pipeline_http_client.py
```
也支持rpc的方式
```
python3 pipeline_rpc_client.py
```
输出
```
{'err_no': 0, 'err_msg': '', 'key': ['res'], 'value': ["['土地整治与土壤修复研究中心', '华南农业大学1素图']"]}
```
