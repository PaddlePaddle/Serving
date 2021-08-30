# Fit a line prediction example

([简体中文](./README_CN.md)|English)

## Get data

```shell
sh get_data.sh
```



## RPC service

### Start server

```shell
python3 -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9393
```

## Client prediction

### RPC Client
The `paddlepaddle` package is used in `test_client.py`, and you may need to download the corresponding package(`pip3 install paddlepaddle`).

``` shell
python3 test_client.py uci_housing_client/serving_client_conf.prototxt
```

### Http Client

``` shell
python3 test_httpclient.py uci_housing_client/serving_client_conf.prototxt
```


## Benchmark
``` shell
bash benchmark.sh uci_housing_model uci_housing_client
```
The log file of benchmark named `profile_log_uci_housing_model`
