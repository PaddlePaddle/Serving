# Imagenet Pipeline WebService

这里以 Imagenet 服务为例来介绍 Pipeline WebService 的使用。

## 获取模型
```
python -m paddle_serving_app.package --get_model resnet_v2_50_imagenet
tar -xzvf resnet_v2_50_imagenet.tar.gz
```

## 启动服务

```
python resnet50_web_service.py &>log.txt &
```

## 测试
```
python pipeline_rpc_client.py
```

