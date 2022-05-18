# 保存用于 Serving 部署的模型参数

- [背景介绍](#1)
- [功能设计](#2)
- [功能使用](#3)
  - [PYTHON 命令执行](#3.1)
  - [代码引入执行](#3.2)
- [Serving 部署](#4)
  - [服务端部署示例](#4.1)
  - [客户端部署示例](#4.2)

<a name="1"></a>

## 背景介绍

模型参数信息保存在模型文件中，为什么还要保存用于 Paddle Serving 部署的模型参数呢，原因有3个：

1. 服务化场景分为客户端和服务端，服务端加载模型，而在客户端没有模型信息，但需要在客户端需实现数据拼装和类型转换。
2. 模型升级过程中 `feed vars` 和 `fetch vars` 的名称变化会导致代码升级，通过增加一个 `alias_name` 字段映射名称，代码无需升级。
3. 部署 `Web` 服务，并使用 `URL` 方式访问时，请求信息中缺少类型和维度信息，在服务端推理前需要进行转换。

<a name="2"></a>

## 功能设计

飞桨训推一体框架中，从动态图模型训练到静态图推理部署，一体化流程如下所示
```
①动态图训练 → ②模型动转静 → ③静态模型 → ④模型保存 → ⑤Serving 部署
```
在飞桨框架2.1对模型与参数的保存与载入相关接口进行了梳理，完整文档参考[模型保存与载入](https://www.paddlepaddle.org.cn/documentation/docs/zh/guides/02_paddle2.0_develop/08_model_save_load_cn.html)
- 对于训练调优场景，我们推荐使用 `paddle.save/load` 保存和载入模型；
- 对于推理部署场景，我们推荐使用 `paddle.jit.save/load`（动态图）和 `paddle.static.save/load_inference_model` （静态图）保存载入模型；

Paddle Serving 模型参数保存接口定位是在 `②模型动转静` 导出 `③静态模型`后，使用 `paddle.static.load_inference_model` 接口加载模型，和 `paddle.static.save_vars` 接口保存模型参数。

生成的模型参数信息保存在 `paddle_serving_server/client.prototxt` 文件中，其格式如下
```
feed_var {
  name: "x"
  alias_name: "image"
  is_lod_tensor: false
  feed_type: 1
  shape: 3
  shape: 960
  shape: 960
}
fetch_var {
  name: "save_infer_model/scale_0.tmp_1"
  alias_name: "save_infer_model/scale_0.tmp_1"
  is_lod_tensor: false
  fetch_type: 1
  shape: 1
  shape: 960
  shape: 960
}
```

| 参数 |   描述 |
|------|---------|
| name | 实际变量名  |
| alias_name | 变量别名，与 name 的关联业务场景中变量名 |
| is_lod_tensor | 是否为 LOD Tensor |
| feed_type | feed 变量类型|
| fetch_type | fetch 变量类型|
| shape 数组 | 变量的 Shape 信息 |

feed 与 fetch 变量的类型列表如下:
| 类型 | 类型值 |
|------|------|
| int64 | 0   |
| float32  |1 |
| int32 | 2 |
| float64 | 3 |
| int16 | 4 |
| float16 | 5 |
| bfloat16 | 6 |
| uint8 | 7 |
| int8 | 8 |
| bool | 9 |
| complex64 | 10 
| complex128 | 11 |

<a name="3"></a>

## 功能使用

Paddle 推理模型有3种形式，每种形式的读模型的方式都不同，散列方式必须以路径方式加载，其余2种采用目录或文件方式均可。
1) Paddle 2.0前版本：`__model__`, `__params__`
2) Paddle 2.0后版本：`*.pdmodel`, `*.pdiparams`
3) 散列：`__model__`, `conv2d_1.w_0`, `conv2d_2.w_0`, `fc_1.w_0`, `conv2d_1.b_0`, ... 

`paddle_serving_client.convert` 接口既支持 PYTHON 命令方式执行，又支持 代码中引入运行。


| 参数 | 类型 | 默认值 | 描述 |
|--------------|------|-----------|--------------------------------|
| `dirname` | str | - | 需要转换的模型文件存储路径，Program结构文件和参数文件均保存在此目录。|
| `serving_server` | str | `"serving_server"` | 转换后的模型文件和配置文件的存储路径。默认值为serving_server |
| `serving_client` | str | `"serving_client"` | 转换后的客户端配置文件存储路径。默认值为serving_client |
| `model_filename` | str | None | 存储需要转换的模型Inference Program结构的文件名称。如果设置为None，则使用 `__model__` 作为默认的文件名 |
| `params_filename` | str | None | 存储需要转换的模型所有参数的文件名称。当且仅当所有模型参数被保>存在一个单独的二进制文件中，它才需要被指定。如果模型参数是存储在各自分离的文件中，设置它的值为None |

<a name="3.1"></a>

**一.PYTHON 命令执行**

首先需要安装 `paddle_serivng_client` 包，以目录方式加载模型。

示例一，是以模型路径方式加载模型，适用于全部3种类型。
```python
python3 -m paddle_serving_client.convert --dirname ./your_inference_model_dir
```

示例二，以指定加载 `当前路径` 下模型 `dygraph_model.pdmodel` 和 `dygraph_model.pdiparams`，保存结果在 `serving_server` 和 `serving_client` 目录。
```python
python3 -m paddle_serving_client.convert --dirname . --model_filename dygraph_model.pdmodel --params_filename dygraph_model.pdiparams --serving_server serving_server --serving_client serving_client
```

<a name="3.2"></a>

**二.代码引入执行**

代码引入执行方式，通过 `import io` 包并调用 `inference_model_to_serving` 实现模型参数保存。
```python
import paddle_serving_client.io as serving_io
serving_io.inference_model_to_serving(dirname, serving_server="serving_server", serving_client="serving_client",  model_filename=None, params_filename=None)
```

<a name="4"></a>

## Serving 部署
生成完的模型可直接用于服务化推理，服务端使用和客户端使用。

<a name="4.1"></a>

**一.服务端部署示例**

示例一：C++ Serving 启动服务
```
python3 -m paddle_serving_server.serve --model serving_server --port 9393 --gpu_id 0
```

示例二：Python Pipeline 启动服务，在 `config.yml` 中指定模型路径
```
op:
    det:
        #并发数，is_thread_op=True时，为线程并发；否则为进程并发
        concurrency: 6

        #当op配置没有server_endpoints时，从local_service_conf读取本地服务配置
        local_service_conf:
            #client类型，包括brpc, grpc和local_predictor.local_predictor不启动Serving服务，进程内预测
            client_type: local_predictor

            #det模型路径
            model_config: ocr_det_model

            #Fetch结果列表，以client_config中fetch_var的alias_name为准
            fetch_list: ["save_infer_model/scale_0.tmp_1"]

            # device_type, 0=cpu, 1=gpu, 2=tensorRT, 3=arm cpu, 4=kunlun xpu
            device_type: 0
```

<a name="4.2"></a>

**二.客户端部署示例**

通过 `client` 对象的 `load_client_config` 接口加载模型配置信息
```
from paddle_serving_client import Client
from paddle_serving_app.reader import Sequential, File2Image, Resize, CenterCrop
from paddle_serving_app.reader import RGB2BGR, Transpose, Div, Normalize

client = Client()
client.load_client_config(
    "serving_client/serving_client_conf.prototxt")
client.connect(["127.0.0.1:9393"])

seq = Sequential([
    File2Image(), Resize(256), CenterCrop(224), RGB2BGR(), Transpose((2, 0, 1)),
    Div(255), Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225], True)
])

image_file = "daisy.jpg"
img = seq(image_file)
fetch_map = client.predict(feed={"inputs": img}, fetch=["save_infer_model/scale_0.tmp_0"])
```
