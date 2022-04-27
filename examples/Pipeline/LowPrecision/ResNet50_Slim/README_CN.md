# Python Pipeline 低精度部署案例

这里以 ResNet50 量化模型为例，介绍 Python Pipline 低精度量化模型部署案例。

## 1.获取模型
```
wget https://paddle-inference-dist.bj.bcebos.com/inference_demo/python/resnet50/ResNet50_quant.tar.gz
tar zxvf ResNet50_quant.tar.gz
```

## 2.保存模型参数
```
python3 -m paddle_serving_client.convert --dirname ResNet50_quant --serving_server serving_server --serving_client serving_client
```

## 3.启动服务
```
python3 resnet50_web_service.py &>log.txt &
```

## 4.测试
```
python3 pipeline_rpc_client.py
python3 pipeline_http_client.py
```
