
## Prepare
### convert model
```
python3 -m paddle_serving_client.convert --dirname erine
```

## RPC Service

### Start Service

```
python3 -m paddle_serving_server.serve --model serving_server --port 7704 --use_lite --use_xpu --ir_optim
```

### Client Prediction

```
head data-c.txt | python3 ernie_client.py --model serving_client/serving_client_conf.prototxt
```
