# 线性回归预测服务示例

## 获取数据

```shell
sh get_data.sh
```

## 开启 gRPC 服务端

``` shell
python test_server.py uci_housing_model/
```

也可以通过下面的一行代码开启默认 gRPC 服务：

```shell
python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9393 --use_multilang
```

## 客户端预测

### 同步预测

``` shell
python test_sync_client.py
```

### 异步预测

``` shell
python test_asyn_client.py
```

### Batch 预测

``` shell
python test_batch_client.py
```

### 预测超时

``` shell
python test_timeout_client.py
```
