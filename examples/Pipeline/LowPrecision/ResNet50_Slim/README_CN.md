# Imagenet Pipeline WebService

这里以 Imagenet 服务为例来介绍 Pipeline WebService 的使用。

## 获取模型
```
wget https://paddle-inference-dist.bj.bcebos.com/inference_demo/python/resnet50/ResNet50_quant.tar.gz
tar zxvf ResNet50_quant.tar.gz
```

## 启动服务

```
python3 resnet50_web_service.py &>log.txt &
```

## 测试
```
python3 pipeline_rpc_client.py
```
