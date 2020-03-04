## 图像分类示例

示例中采用ResNet50_vd模型执行imagenet 1000分类任务。

### 模型及配置文件获取
```
wget https://paddle-serving.bj.bcebos.com/imagenet-example/conf_and_model.tar.gz
tar -xzvf conf_and_model.tar.gz
```
### 执行预测服务

启动server端
```
python image_classification_service.py conf_and_model/serving_server_model workdir
```

client端进行预测
```
python image_http_client.py
```

*server端示例中服务端口为9393端口，client端示例中数据来自./data文件夹，server端地址为本地9393端口，可根据实际情况更改脚本。
