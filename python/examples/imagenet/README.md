## 图像分类示例

示例中采用ResNet50_vd模型执行imagenet 1000分类任务。

### 模型及配置文件获取
```
sh get_model.sh
```
### 执行wb service预测服务

启动server端
```
python image_classification_service.py conf_and_model/serving_server_model workdir 9393
```

client端进行预测
```
python image_http_client.py
```
### 执行rpc service预测服务

启动server端
```
python image_server.py conf_and_model/serving_server_model workdir 9393
```

client端进行预测
```
python image_rpc_client.py conf_and_model/serving_client_conf/serving_client_conf.prototxt
```
*server端示例中服务端口为9393端口，client端示例中数据来自./data文件夹，server端地址为本地9393端口，可根据实际情况更改脚本。*
