## Paddle Serving低精度部署

(简体中文|[English](./Low_Precision_EN.md))

低精度部署, 在Intel CPU上支持int8、bfloat16模型，Nvidia TensorRT支持int8、float16模型。

## C++ Serving 部署量化模型

### 通过PaddleSlim量化生成低精度模型
详细见[PaddleSlim量化](https://paddleslim.readthedocs.io/zh_CN/latest/tutorials/quant/overview.html)

### 使用TensorRT int8加载PaddleSlim Int8量化模型进行部署
首先下载Resnet50 [PaddleSlim量化模型](https://paddle-inference-dist.bj.bcebos.com/inference_demo/python/resnet50/ResNet50_quant.tar.gz),并转换为Paddle Serving支持的部署模型格式。
```
wget https://paddle-inference-dist.bj.bcebos.com/inference_demo/python/resnet50/ResNet50_quant.tar.gz
tar zxvf ResNet50_quant.tar.gz

python -m paddle_serving_client.convert --dirname ResNet50_quant
```
启动rpc服务, 设定所选GPU id、部署模型精度
```
python -m paddle_serving_server.serve --model serving_server --port 9393 --gpu_ids 0 --use_trt --precision int8 
```
使用client进行请求
```
from paddle_serving_client import Client
from paddle_serving_app.reader import Sequential, File2Image, Resize, CenterCrop
from paddle_serving_app.reader import RGB2BGR, Transpose, Div, Normalize

client = Client()
client.load_client_config(
    "resnet_v2_50_imagenet_client/serving_client_conf.prototxt")
client.connect(["127.0.0.1:9393"])

seq = Sequential([
    File2Image(), Resize(256), CenterCrop(224), RGB2BGR(), Transpose((2, 0, 1)),
    Div(255), Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225], True)
])

image_file = "daisy.jpg"
img = seq(image_file)
fetch_map = client.predict(feed={"image": img}, fetch=["score"])
print(fetch_map["score"].reshape(-1))
```

## Python Pipeline 部署量化模型

请参考 [Python Pipeline 低精度推理](./Python_Pipeline/Pipeline_Features_CN.md#低精度推理)


## 参考文档
* [PaddleSlim](https://github.com/PaddlePaddle/PaddleSlim)
* PaddleInference Intel CPU部署量化模型[文档](https://paddle-inference.readthedocs.io/en/latest/optimize/paddle_x86_cpu_int8.html)
* PaddleInference NV GPU部署量化模型[文档](https://paddle-inference.readthedocs.io/en/latest/optimize/paddle_trt.html)
