# 图像分类

## 获取模型

```
python3 -m paddle_serving_app.package --get_model mobilenet_v2_imagenet
tar -xzvf mobilenet_v2_imagenet.tar.gz
```

## RPC 服务

### 启动服务端

```
python3 -m paddle_serving_server.serve --model mobilenet_v2_imagenet_model --gpu_ids 0 --port 9393
```

### 客户端预测

```
python3 mobilenet_tutorial.py
```
