# 线性回归预测服务示例

(简体中文|[English](./README.md))

## 获取数据

```shell
sh get_data.sh
```




## 开启服务端

```shell
python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9393
```

## 客户端预测

### 客户端RPC

`test_client.py`中使用了`paddlepaddle`包，需要进行下载（`pip install paddlepaddle`）。

``` shell
python test_client.py uci_housing_client/serving_client_conf.prototxt
```

### 客户端Http预测

``` shell
python test_httpclient.py uci_housing_client/serving_client_conf.prototxt
```


## 性能测试
``` shell
bash benchmark.sh uci_housing_model uci_housing_client
```
性能测试的日志文件为profile_log_uci_housing_model

如需修改性能测试用例的参数，请修改benchmark.sh中的配置信息。

注意:uci_housing_model和uci_housing_client路径后不要加'/'符号,示例需要在GPU机器上运行。
