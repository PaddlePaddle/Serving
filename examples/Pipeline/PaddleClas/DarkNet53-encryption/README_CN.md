# Imagenet Pipeline WebService

这里以 Imagenet 服务为例来介绍 Pipeline WebService 的使用。

## 获取模型
```
sh get_model.sh
python encrypt.py
```

## 启动服务

```
python -m paddle_serving_server.serve --model encrypt_server/ --port 9400  --encryption_rpc_port 9401  --use_encryption_model &
python web_service.py &>log.txt &
```

## 测试
```
python http_client.py
```
如果您已经配置好了api gateway， 您可以使用 `https_client.py`

~
