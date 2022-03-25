# 图像分割

## 获取模型

```
python3 -m paddle_serving_app.package --get_model unet
tar -xzvf unet.tar.gz
```

## RPC 服务

### 启动服务端

```
python3 -m paddle_serving_server.serve --model unet_model --gpu_ids 0 --port 9494
```

### 客户端预测

```
python3 seg_client.py
```
