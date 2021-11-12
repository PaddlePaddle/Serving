
## Prepare
### download model and extract
```
wget https://paddle-inference-dist.bj.bcebos.com/PaddleLite/models_and_data_for_unittests/VGG19.tar.gz
tar zxvf VGG19.tar.gz
```
### convert model
```
python3 -m paddle_serving_client.convert --dirname VGG19
```
### or, you can get the serving saved model directly
```
wget https://paddle-serving.bj.bcebos.com/models/xpu/vgg19.tar.gz
tar zxvf vgg19.tar.gz 
```

## RPC Service

### Start Service

```
python3 -m paddle_serving_server.serve --model serving_server --port 7702 --use_lite --use_xpu --ir_optim
```

### Client Prediction

```
python3 vgg19_client.py
```
