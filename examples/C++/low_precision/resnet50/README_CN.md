# resnet50 int8示例
(简体中文|[English](./README.md))

## 通过PaddleSlim量化生成低精度模型
详细见[PaddleSlim量化](https://paddleslim.readthedocs.io/zh_CN/latest/tutorials/quant/overview.html)

## 使用TensorRT int8加载PaddleSlim Int8量化模型进行部署
首先下载Resnet50 [PaddleSlim量化模型](https://paddle-inference-dist.bj.bcebos.com/inference_demo/python/resnet50/ResNet50_quant.tar.gz),并转换为Paddle Serving支持的部署模型格式。
```
wget https://paddle-inference-dist.bj.bcebos.com/inference_demo/python/resnet50/ResNet50_quant.tar.gz
tar zxvf ResNet50_quant.tar.gz

python3 -m paddle_serving_client.convert --dirname ResNet50_quant
```
启动rpc服务, 设定所选GPU id、部署模型精度
```
python3 -m paddle_serving_server.serve --model serving_server --port 9393 --gpu_ids 0 --use_trt --precision int8 
```
使用client进行请求
```
python3 resnet50_client.py
```

## 参考文档
* [PaddleSlim](https://github.com/PaddlePaddle/PaddleSlim)
* PaddleInference Intel CPU部署量化模型[文档](https://paddle-inference.readthedocs.io/en/latest/optimize/paddle_x86_cpu_int8.html)
* PaddleInference NV GPU部署量化模型[文档](https://paddle-inference.readthedocs.io/en/latest/optimize/paddle_trt.html)
