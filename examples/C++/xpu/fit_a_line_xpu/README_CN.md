# 线性回归预测服务示例

(简体中文|[English](./README.md))

## 获取数据

```shell
sh get_data.sh
```



## RPC服务

### 开启服务端

``` shell
python3 test_server.py uci_housing_model/
```

也可以通过下面的一行代码开启默认RPC服务：

```shell
python3 -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9393 --use_lite --use_xpu --ir_optim
```

### 客户端预测

`test_client.py`中使用了`paddlepaddle`包，需要进行下载（`pip3 install paddlepaddle`）。

``` shell
python3 test_client.py uci_housing_client/serving_client_conf.prototxt
```
