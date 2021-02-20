# 线性回归预测服务示例

(简体中文|[English](./README.md))

## 获取数据

```shell
sh get_data.sh
```



## RPC服务

### 开启服务端

``` shell
python test_server.py uci_housing_model/
```

也可以通过下面的一行代码开启默认RPC服务：

```shell
python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9393 --use_lite --use_xpu --ir_optim
```

### 客户端预测

`test_client.py`中使用了`paddlepaddle`包，需要进行下载（`pip install paddlepaddle`）。

``` shell
python test_client.py uci_housing_client/serving_client_conf.prototxt
```



## HTTP服务

### 开启服务端

通过下面的一行代码开启默认web服务：

``` shell
python test_server.py
```

### 客户端预测

``` shell
curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"x": [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]}], "fetch":["price"]}' http://127.0.0.1:9292/uci/prediction
```
