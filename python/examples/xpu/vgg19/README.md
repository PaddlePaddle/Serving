
## Prepare
### convert model
```
python -m paddle_serving_client.convert --dirname VGG19
```

## RPC Service

### Start Service

```
python -m paddle_serving_server.serve --model serving_server --port 7702 --use_lite --use_xpu --ir_optim
```

### Client Prediction

```
python vgg19_client.py
```
