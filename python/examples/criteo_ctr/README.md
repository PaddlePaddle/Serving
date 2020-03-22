## CTR预测服务

### 获取样例数据
```
sh get_data.sh
```

### 保存模型和配置文件
```
python local_train.py
```
执行脚本后会在当前目录生成serving_server_model和serving_client_config文件夹。

### 启动RPC预测服务

```
python -m paddle_serving_server.serve --model ctr_serving_model/ --port 9292
```

### 执行预测

```
python test_client.py ctr_client_conf/serving_client_conf.prototxt raw_data/
```
