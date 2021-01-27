# Fit a line prediction example

([简体中文](./README_CN.md)|English)

## Get data

```shell
sh get_data.sh
```



## RPC service

### Start server

```shell
python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9393
```

### Client prediction

The `paddlepaddle` package is used in `test_client.py`, and you may need to download the corresponding package(`pip install paddlepaddle`).

``` shell
python test_client.py uci_housing_client/serving_client_conf.prototxt
```



## HTTP service

### Start server

Start a web service with default web service hosting modules:
``` shell
python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9393 --name uci
```

### Client prediction

``` shell
curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"x": [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]}], "fetch":["price"]}' http://127.0.0.1:9393/uci/prediction
```
