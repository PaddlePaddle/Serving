## 带稀疏参数索引服务的CTR预测服务

该样例是为了展示gRPC Server 端 `load_model_config` 函数，在这个例子中，bRPC Server 端与 bRPC Client 端的配置文件是不同的（bPRC Client 端的数据先交给 cube，经过 cube 处理后再交给预测库）

### 获取样例数据
```
sh get_data.sh
```

### 下载模型和稀疏参数序列文件
```
wget https://paddle-serving.bj.bcebos.com/unittest/ctr_cube_unittest.tar.gz
tar xf ctr_cube_unittest.tar.gz
mv models/ctr_client_conf ./
mv models/ctr_serving_model_kv ./
mv models/data ./cube/
```
执行脚本后会在当前目录有ctr_server_model_kv和ctr_client_config文件夹。

### 启动稀疏参数索引服务
```
wget https://paddle-serving.bj.bcebos.com/others/cube_app.tar.gz
tar xf cube_app.tar.gz
mv cube_app/cube* ./cube/
sh cube_prepare.sh &
```

此处，模型当中的稀疏参数会被存放在稀疏参数索引服务Cube当中，关于稀疏参数索引服务Cube的介绍，请阅读[稀疏参数索引服务Cube单机版使用指南](../../../doc/CUBE_LOCAL_CN.md)

### 启动RPC预测服务，服务端线程数为4（可在test_server.py配置）

```
python test_server.py ctr_serving_model_kv ctr_client_conf/serving_client_conf.prototxt 
```

### 执行预测

```
python test_client.py ./raw_data
```
