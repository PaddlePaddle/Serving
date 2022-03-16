# 文字识别（OCR）部署案例

## 文字识别模型介绍

OCR 技术一般指光学字符识别。 OCR（Optical Character Recognition，光学字符识别）是指电子设备（例如扫描仪或数码相机）检查纸上打印的字符，通过检测暗、亮的模式确定其形状，然后用字符识别方法将形状翻译成计算机文字的过程。

[PaddleOCR](https://github.com/PaddlePaddle/PaddleOCR) 是百度飞桨 OCR 模型套件库，旨在打造一套丰富、领先、且实用的 OCR 工具库，助力开发者训练出更好的模型，并应用落地。具有 PP-OCR 系列高质量预训练模型，准确的识别效果；支持中英文数字组合识别、竖排文本识别、长文本识别；支持多语言识别：韩语、日语、德语、法语等约80种语言等特性。

PaddleOCR 提供的 PP-OCR 系列模型覆盖轻量级服务端、轻量级移动端和通用服务端3种场景。

| 模型介绍 | 模型大小 | 模型名称 | 推荐场景 | 
| ------- | ------ | ----- | ----- |
| 中英文超轻量模型 | 13.0M | ch_PP-OCRv2_xx	| 服务器端 或 移动端 |
| 中英文超轻量移动端模型 | 9.4M | ch_ppocr_mobile_v2.0_xx | 移动端|
| 中英文通用服务端模型 | 143.4M | ch_ppocr_server_v2.0_xx | 服务器端 |

## 服务化部署案例

本章节选用中英文超轻量模型 ch_PP-OCRv2_xx 制作部署案例，模型体积小，效果很好，属于性价比很高的选择。 

Paddle Serving 提供2种 OCR 的部署案例，分别是基于 Python Pipeline 和 C++ Serving 实现。 Python Pipeline 框架使用 Python 语言开发，具备更好的易用性。C++ Serving 框架使用 C++ 开发。本文介绍基于 Python Pipeline 的部署案例。

**Python Pipeline 部署案例**

前提条件是你已完成[环境安装]()步骤，并已验证环境安装成功，此处不在赘述。

在克隆 Serving 代码后，进入 examples/Pipeline/PaddleOCR/ocr 目录下，包括程序、配置和性能测试脚本。
```
git clone https://github.com/PaddlePaddle/Serving
```

#### 1.获取模型

为了节省大家的时间，已将预训练模型使用[保存模型服务化参数]()方法打包成压缩包，下载并解压即可使用。如你自训练的模型需经过保存模型服务化参数步骤才能服务化部署。
```
python3 -m paddle_serving_app.package --get_model ocr_rec
tar -xzvf ocr_rec.tar.gz
python3 -m paddle_serving_app.package --get_model ocr_det
tar -xzvf ocr_det.tar.gz
```

#### 2.下载测试数据集（可选）
提供1套测试图片集，或者使用自有数据集，忽略此步骤。
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/ocr/test_imgs.tar
tar xf test_imgs.tar
```

#### 3.修改 Config.yml 配置（可选）
由于配置项较多，每个选择已有详细中文说明，因此，仅重点介绍部分核心选项的使用。
- dag: 表示有向无环图级别的选项，如图中 op 节点资源类型，推荐使用进程模型，即 False
  - tracer: 统计各个阶段耗时、channel堆积信息记录在 PipelineServingLogs/pipeline.tracer 文件中
- op: 表示模型或处理函数，如此配置中定义了 det 和 rec 2个模型
  - concurrency: 是进程或线程的并发数，可实现单机多卡推理。 
  - local_service_conf: 本地推理（非远端 RPC 推理），推荐使用 local_predictor，性能好。
    - model_config: 模型路径
    - fetch_list: 指定返回模型的某个 fetch var，注释（不填）此项时，为返回所有 fetch var。
    - device_type: 计算硬件类型
    - devices: 设备ID，可以设置多张卡，如"0, 1, 2"
    - thread_num: 为 CPU 推理时，数学计算线程数，开大可降低处理时长
    - ir_optim: IR 优化，开启 TensorRT 时，必须开启 ir_optim


#### 4. 多模型组合
每个 op (模型或函数) 处理和 图结构 定义在 web_service.py 程序中。
```
# DetOp 继承于 Class Op 并重写前后处理
class DetOp(Op):
    def init_op(self):
    def preprocess(self, input_dicts, data_id, log_id):
    def postprocess(self, input_dicts, fetch_dict, data_id, log_id):

# 图结构由在 OcrService::get_pipeline_response 函数中定义，并通过 input_ops 参数设定 Op 关系。
class OcrService(WebService):
    def get_pipeline_response(self, read_op):
        det_op = DetOp(name="det", input_ops=[read_op])
        rec_op = RecOp(name="rec", input_ops=[det_op])
        return rec_op
```

#### 5.启动服务 
启动服务前，要确认 URL://{ip}:{port}/{name}/{method} 。本项目中 {name} 即是 web_service.py 中 OcrService name 参数 "ocr"。 {method} 默认为 "prediction"

```
ocr_service = OcrService(name="ocr")
```

之后，启动服务和测试客户端请求
```
# Run Server
python3 web_service.py &>log.txt &

# Run Client
python3 pipeline_http_client.py
```

模型效果：

<p align="center">
  <img src="https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.3/doc/imgs_results/PP-OCRv2/PP-OCRv2-pic003.jpg?raw=true" width="345"/> 
</p>
