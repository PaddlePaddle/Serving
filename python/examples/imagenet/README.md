## 图像分类示例

示例中采用ResNet50_vd模型执行imagenet 1000分类任务。

### 获取模型配置文件和样例数据
```
sh get_model.sh
```
### 执行HTTP预测服务

启动server端
```
python image_classification_service.py ResNet50_vd_model workdir 9393 #cpu预测服务
```
```
python image_classification_service_gpu.py ResNet50_vd_model workdir 9393 #gpu预测服务
```


client端进行预测
```
python image_http_client.py
```
### 执行RPC预测服务

启动server端
```
python -m paddle_serving_server.serve --model ResNet50_vd_model --port 9393 #cpu预测服务
```

```
python -m paddle_serving_server_gpu.serve --model ResNet50_vd_model --port 9393 --gpu_ids 0 #gpu预测服务
```

client端进行预测
```
python image_rpc_client.py ResNet50_vd_client_config/serving_client_conf.prototxt
```
*server端示例中服务端口为9393端口，client端示例中数据来自./data文件夹，server端地址为本地9393端口，可根据实际情况更改脚本。*
