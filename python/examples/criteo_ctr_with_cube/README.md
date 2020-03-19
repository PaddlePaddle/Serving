## 带稀疏参数服务器的CTR预测服务

### 获取样例数据
```
sh get_data.sh
```

### 保存模型和配置文件
```
python local_train.py
```
执行脚本后会在当前目录生成ctr_server_model和ctr_client_config文件夹,以及ctr_server_model_kv, ctr_client_conf_kv。

### 启动稀疏参数服务器
```
cp ../../../build_server/core/predictor/seq_generator seq_generator
cp ../../../build_server/output/bin/cube* ./cube/
cp ../../../build_server/core/cube/cube-api/cube-cli ./cube/
sh cube_prepare.sh &
```

### 启动RPC预测服务

```
python test_server.py ctr_serving_model_kv 
```

### 执行预测

```
python test_client.py ctr_client_conf/serving_client_conf.prototxt ./raw_data
```

### Benchmark

设备 ：Intel(R) Xeon(R) CPU E5-2640 v3 @ 2.60GHz 

模型 ：[Criteo CTR](https://github.com/PaddlePaddle/Serving/blob/develop/python/examples/ctr_criteo_with_cube/network_conf.py)

server thread num ： 16

执行
```
bash benchmark.sh
```
客户端每个线程会发送1000个batch

| client  thread num | prepro | client infer | op0    | op1   | op2    | postpro | avg_latency | qps   |
| ------------------ | ------ | ------------ | ------ | ----- | ------ | ------- | ----- | ----- |
| 1                  | 0.035  | 1.596        | 0.021  | 0.518 | 0.0024 | 0.0025  | 6.774 | 147.7 |
| 2                  | 0.034  | 1.780        | 0.027  | 0.463 | 0.0020 | 0.0023  | 6.931 | 288.3 |
| 4                  | 0.038  | 2.954        | 0.025  | 0.455 | 0.0019 | 0.0027  | 8.378 | 477.5 |
| 8                  | 0.044  | 8.230        | 0.028  | 0.464 | 0.0023 | 0.0034  | 14.191 | 563.8 |
| 16                 | 0.048  | 21.037       | 0.028  | 0.455 | 0.0025 | 0.0041  | 27.236 | 587.5 |

平均每个线程耗时图如下

![avg cost](../../../doc/criteo-cube-benchmark-avgcost.png)

每个线程QPS耗时如下

![qps](../../../doc/criteo-cube-benchmark-qps.png)
