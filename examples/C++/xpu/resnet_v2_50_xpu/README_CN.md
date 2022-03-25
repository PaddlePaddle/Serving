# 图像分类

## 获取模型

```
python3 -m paddle_serving_app.package --get_model resnet_v2_50_imagenet
tar -xzvf resnet_v2_50_imagenet.tar.gz
```

## RPC 服务

### 启动服务端

```
python3 -m paddle_serving_server.serve --model resnet_v2_50_imagenet_model --port 9393 --use_lite --use_xpu --ir_optim
```

### 客户端预测

```
python3 resnet50_client.py
```
