# Fit a line prediction example

([简体中文](./README_CN.md)|English)

## Get data

```shell
sh get_data.sh
```

## RPC service

### Start server
You can use the following code to start the RPC service 
```shell
python3 -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9393 --use_lite --use_xpu --ir_optim
```

### Client prediction

The `paddlepaddle` package is used in `test_client.py`, and you may need to download the corresponding package(`pip3 install paddlepaddle`).

``` shell
python3 test_client.py uci_housing_client/serving_client_conf.prototxt
```
