# Imagenet Pipeline WebService

这里以 Imagenet 服务为例来介绍 Pipeline WebService 的使用。

## 获取模型
```
sh get_model.sh
```

## 启动服务

```
python3 resnet50_web_service.py &>log.txt &
```

## 测试
```
python3 pipeline_rpc_client.py
```
