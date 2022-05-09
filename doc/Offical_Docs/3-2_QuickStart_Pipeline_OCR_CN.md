# Python Pipeline 快速部署案例

- [模型介绍](#1)
- [部署步骤](#2)
    - [获取模型与保存模型参数](#2.1)
    - [保存 Serving 部署的模型参数](#2.2)
    - [下载测试数据集（可选）](#2.3)
    - [修改配置文件（可选）](#2.4)
    - [代码与配置信息绑定](#2.5)
    - [启动服务与验证](#2.6)


Python Pipeline 框架使用 Python 语言开发，是一套端到端多模型组合服务编程框架，旨在降低编程门槛，提高资源使用率（尤其是GPU设备），提升整体服务的预估效率。详细设计参考[ Python Pipeline 设计与使用]()

<a name="1"></a>

## 模型介绍

OCR 技术一般指光学字符识别。 OCR（Optical Character Recognition，光学字符识别）是指电子设备（例如扫描仪或数码相机）检查纸上打印的字符，通过检测暗、亮的模式确定其形状，然后用字符识别方法将形状翻译成计算机文字的过程。

[PaddleOCR](https://github.com/PaddlePaddle/PaddleOCR) 是百度飞桨 OCR 模型套件库，旨在打造一套丰富、领先、且实用的 OCR 工具库，助力开发者训练出更好的模型，并应用落地。具有 PP-OCR 系列高质量预训练模型，准确的识别效果；支持中英文数字组合识别、竖排文本识别、长文本识别；支持多语言识别：韩语、日语、德语、法语等约80种语言等特性。

PaddleOCR 提供的 PP-OCR 系列模型覆盖轻量级服务端、轻量级移动端和通用服务端3种场景。

| 模型介绍 | 模型大小 | 模型名称 | 推荐场景 | 
| ------- | ------ | ----- | ----- |
| 中英文超轻量模型 | 13.0M | ch_PP-OCRv2_xx	| 服务器端 或 移动端 |
| 中英文超轻量移动端模型 | 9.4M | ch_ppocr_mobile_v2.0_xx | 移动端|
| 中英文通用服务端模型 | 143.4M | ch_ppocr_server_v2.0_xx | 服务器端 |

<a name="2"></a>

## 部署步骤

前提条件是你已完成[环境安装]()步骤，并已验证环境安装成功，此处不在赘述。

在克隆 Serving 代码后，进入 examples/Pipeline/PaddleOCR/ocr 目录下，包括程序、配置和性能测试脚本。
```
git clone https://github.com/PaddlePaddle/Serving
```
通过6个步骤操作即可实现 OCR 示例部署。
- 一.获取模型
- 二.保存 Serving 部署的模型参数
- 三.下载测试数据集（可选）
- 四.修改 `config.yml` 配置（可选）
- 五.代码与配置信息绑定
- 六.启动服务与验证

<a name="2.1"></a>

**一.获取模型与保存模型参数**

本章节选用中英文超轻量模型 ch_PP-OCRv2_xx 制作部署案例，模型体积小，效果很好，属于性价比很高的选择。 

```
python3 -m paddle_serving_app.package --get_model ocr_rec
tar -xzvf ocr_rec.tar.gz
python3 -m paddle_serving_app.package --get_model ocr_det
tar -xzvf ocr_det.tar.gz
```

<a name="2.2"></a>

**二.保存 Serving 部署的模型参数**

为了节省大家的时间，已将预训练模型使用[保存用于 Serving 部署的模型参数](./5-1_Save_Model_Params_CN.md)方法打包成压缩包，下载并解压即可使用。如你自训练的模型需经过保存模型服务化参数步骤才能服务化部署。

<a name="2.3"></a>

**三.下载测试数据集（可选）**

下载测试图片集，如使用自有测试数据集，可忽略此步骤。
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/ocr/test_imgs.tar
tar xf test_imgs.tar
```

<a name="2.4"></a>

**四.修改配置文件（可选）**

修改配置文件 `config.yml` 设置服务、图、OP 级别属性。如果使用默认配置，此步骤可忽略。

由于配置项较多，仅重点介绍部分核心选项的使用，完整配置选项说明可参考[ 配置说明]()
```
#rpc端口, rpc_port和http_port不允许同时为空。当rpc_port为空且http_port不为空时，会自动将rpc_port设置为http_port+1
rpc_port: 18090

#http端口, rpc_port和http_port不允许同时为空。当rpc_port可用且http_port为空时，不自动生成http_port
http_port: 9999

#worker_num, 最大并发数。当build_dag_each_worker=True时, 框架会创建worker_num个进程，每个进程内构建grpcSever和DAG
##当build_dag_each_worker=False时，框架会设置主线程grpc线程池的max_workers=worker_num
worker_num: 20

#build_dag_each_worker, False，框架在进程内创建一条DAG；True，框架会每个进程内创建多个独立的DAG
build_dag_each_worker: false

#有向无环图级别的选项
dag:
    #op资源类型, True, 为线程模型；False，为进程模型
    is_thread_op: False

    #重试次数
    retry: 1

    #使用性能分析, True，生成Timeline性能数据，对性能有一定影响；False为不使用
    use_profile: false

    # 统计各个阶段耗时、Channel在 PipelineServingLogs/pipeline.tracer
    tracer:
        #每次记录的间隔，单位：秒
        interval_s: 10

#模型或可独立控制并发的处理函数级别选项
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

            #计算硬件ID，当devices为""或不写时为CPU预测；当devices为"0", "0,1,2"时为GPU预测，表示使用的GPU卡
            devices: ""

            #use_mkldnn
            #use_mkldnn: True

            #thread_num
            thread_num: 2

            #ir_optim
            ir_optim: True
            
            #开启tensorrt后，进行优化的子图包含的最少节点数
            #min_subgraph_size: 13
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
            fetch_list: ["save_infer_model/scale_0.tmp_1"]

            # device_type, 0=cpu, 1=gpu, 2=tensorRT, 3=arm cpu, 4=kunlun xpu
            device_type: 0

            #计算硬件ID，当devices为""或不写时为CPU预测；当devices为"0", "0,1,2"时为GPU预测，表示使用的GPU卡
            devices: ""

            #use_mkldnn，仅当 CPU 推理时使用
            #use_mkldnn: True

            #thread_num，为 CPU 推理时，数学计算线程数，开大可降低处理时长
            thread_num: 2

            #ir_optim，IR 优化，开启 TensorRT 时，必须开启 ir_optim
            ir_optim: True

            #开启tensorrt后，进行优化的子图包含的最少节点数
            #min_subgraph_size: 3
```

<a name="2.5"></a>

**五.代码与配置信息绑定**

第四步，实现代码和配置文件 Config.yml 绑定，以及设置多模型组合关系。具体包括：

1. 重写模型前后处理：

每个 op (模型或函数) 处理和 图结构 定义在 web_service.py 程序中，本案例实现了 `DetOp` 和 `RecOp` 2个 OP
```
# DetOp 对应配置文件 Config.yml中 det op
class DetOp(Op):
    def init_op(self):
    def preprocess(self, input_dicts, data_id, log_id):
    def postprocess(self, input_dicts, fetch_dict, data_id, log_id):

# RecOp 对应配置文件 Config.yml中 rec op
class RecOp(Op):
    def init_op(self):
    def preprocess(self, input_dicts, data_id, log_id):
    def postprocess(self, input_dicts, fetch_dict, data_id, log_id):

```

2. 构建多模型组合关系

继承父类 `WebService` 派生出 `OcrService` 类，通过重写 `get_pipeline_response()` 接口，实例化 `DetOp` 和 `RecOp` ，`name` 字段与 config.yml 中 op 名称一致； input_ops 是前置 OP 列表实现 多模型组合的图结构。

```
class OcrService(WebService):
    def get_pipeline_response(self, read_op):
        det_op = DetOp(name="det", input_ops=[read_op])
        rec_op = RecOp(name="rec", input_ops=[det_op])
        return rec_op
```

3. 绑定代码与配置文件

通过构造函数 `OcrService(name="ocr")` 设置请求 URL 中 name 字段；通过 `prepare_pipeline_config()` 接口绑定配置文件 `config.yml`；通过 `run_service()` 接口启动服务。

```
ocr_service = OcrService(name="ocr")
ocr_service.prepare_pipeline_config("config.yml")
ocr_service.run_service()
```

<a name="2.6"></a>

**六.启动服务与验证**

启动服务前，可看到程序路径下所有文件路径如下：
```
.
├── 7.jpg
├── benchmark.py
├── benchmark.sh
├── config.yml
├── imgs
│   └── ggg.png
├── ocr_det_client
│   ├── serving_client_conf.prototxt
│   └── serving_client_conf.stream.prototxt
├── ocr_det_model
│   ├── inference.pdiparams
│   ├── inference.pdmodel
│   ├── serving_server_conf.prototxt
│   └── serving_server_conf.stream.prototxt
├── ocr_rec_client
│   ├── serving_client_conf.prototxt
│   └── serving_client_conf.stream.prototxt
├── ocr_rec_model
│   ├── inference.pdiparams
│   ├── inference.pdmodel
│   ├── serving_server_conf.prototxt
│   └── serving_server_conf.stream.prototxt
├── pipeline_http_client.py
├── pipeline_rpc_client.py
├── ppocr_keys_v1.txt
├── ProcessInfo.json
├── README_CN.md
├── README.md
└── web_service.py
```

运行服务程序 `web_service.py` 启动服务端，接收客户端请求，采用图执行引擎执行推理预测。
```
# Run Server
python3 web_service.py &>log.txt &
```

客户端程序 `pipeline_http_client.py` 注册服务端地址，并发送客户端请求。

启动客户端前，要确认 URL://{ip}:{port}/{name}/{method} 。本项目中 {name} 即是 web_service.py 中 OcrService name 参数 "ocr"。 {method} 默认为 "prediction"

```
# Run Client
python3 pipeline_http_client.py
```

模型效果：

<p align="center">
  <img src="https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.3/doc/imgs_results/PP-OCRv2/PP-OCRv2-pic003.jpg?raw=true" width="345"/> 
</p>
