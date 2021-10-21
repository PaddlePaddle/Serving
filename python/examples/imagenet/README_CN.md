## 图像分类示例

(简体中文|[English](./README.md))

示例中采用ResNet50_vd模型执行imagenet 1000分类任务。

### 获取模型配置文件和样例数据
```
sh get_model.sh
```

### 安装数据预处理模块

```
pip3 install paddle_serving_app
```

### 启动服务端（支持BRPC-Client、GRPC-Client、Http-Client）

启动server端
```
python3 -m paddle_serving_server.serve --model ResNet50_vd_model --port 9696 #cpu预测服务
```

```
python3 -m paddle_serving_server.serve --model ResNet50_vd_model --port 9696 --gpu_ids 0 #gpu预测服务
```

### BRPC-Client预测
client端进行预测
```
python3 resnet50_rpc_client.py ResNet50_vd_client_config/serving_client_conf.prototxt
```
*server端示例中服务端口为9696端口


### GRPC-Client/Http-Client预测
client端进行预测
```
python3 resnet50_http_client.py ResNet50_vd_client_config/serving_client_conf.prototxt
```
