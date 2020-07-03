## 图像分类示例

(简体中文|[English](./README.md))

示例中采用ResNet50_vd模型执行imagenet 1000分类任务。

### 获取模型配置文件和样例数据
```
sh get_model.sh
```

### 安装数据预处理模块

```
pip install paddle_serving_app
```

### HTTP服务

启动server端
```
python resnet50_web_service.py ResNet50_vd_model cpu 9696 #cpu预测服务
```
```
python resnet50_web_service.py ResNet50_vd_model gpu 9696 #gpu预测服务
```


发送HTTP POST请求
```
curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"image": "https://paddle-serving.bj.bcebos.com/imagenet-example/daisy.jpg"}], "fetch": ["score"]}' http://127.0.0.1:9696/image/prediction
```

### RPC服务

启动server端
```
python -m paddle_serving_server.serve --model ResNet50_vd_model --port 9696 #cpu预测服务
```

```
python -m paddle_serving_server_gpu.serve --model ResNet50_vd_model --port 9696 --gpu_ids 0 #gpu预测服务
```

client端进行预测
```
python resnet50_rpc_client.py ResNet50_vd_client_config/serving_client_conf.prototxt
```
*server端示例中服务端口为9696端口

### K8S启动

还可以运用K8S启动，在`imagenet_k8s_rpc.yaml`中定义了Serving的Deployment和Service，用户可以在K8S集群上启动Deployment并且通过Service对外暴露服务，用户可以在此基础上进行二次开发。

推荐百度云的[CCE（K8S）集群](https://cloud.baidu.com/search.html?q=CCE)

```
kubectl apply -f imagenet_k8s_rpc.yaml
```
