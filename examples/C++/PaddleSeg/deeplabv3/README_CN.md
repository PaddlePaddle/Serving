# 图像分割

## 获取模型

```
python3 -m paddle_serving_app.package --get_model deeplabv3
tar -xzvf deeplabv3.tar.gz
```

## RPC 服务

### 启动服务端

```
python3 -m paddle_serving_server.serve --model deeplabv3_server --gpu_ids 0 --port 9494
```

### 客户端预测

```
python3 deeplabv3_client.py
