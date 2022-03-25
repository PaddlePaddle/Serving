## CTR预测服务

(简体中文|[English](./README.md))

### 获取样例数据
```
sh get_data.sh
```

### 下载模型
```
wget https://paddle-serving.bj.bcebos.com/criteo_ctr_example/criteo_ctr_demo_model.tar.gz
tar xf criteo_ctr_demo_model.tar.gz
mv models/ctr_client_conf .
mv models/ctr_serving_model .
```
会在当前目录出现`ctr_serving_model` 和 `ctr_client_conf`文件夹。

### 启动RPC预测服务

```
python3 -m paddle_serving_server.serve --model ctr_serving_model/ --port 9292 #启动CPU预测服务
python3 -m paddle_serving_server.serve --model ctr_serving_model/ --port 9292 --gpu_ids 0 #在GPU 0上启动预测服务
```

### 执行预测

```
python3 test_client.py ctr_client_conf/serving_client_conf.prototxt raw_data/part-0
```
预测完毕会输出预测过程的耗时。
