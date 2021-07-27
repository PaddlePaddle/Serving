## Image Classification

([简体中文](./README_CN.md)|English)

The example uses the ResNet50_vd model to perform the imagenet 1000 classification task.

### Get model config and sample dataset
```
sh get_model.sh
```

### Install preprocess module

```
pip install paddle_serving_app
```

### HTTP Service

launch server side
```
python resnet50_web_service.py ResNet50_vd_model cpu 9696 #cpu inference service
```
```
python resnet50_web_service.py ResNet50_vd_model gpu 9696 #gpu inference service
```


client send inference request
```
curl -H "Content-Type:application/json" -X POST -d '{"feed":[{"image": "https://paddle-serving.bj.bcebos.com/imagenet-example/daisy.jpg"}], "fetch": ["score"]}' http://127.0.0.1:9696/image/prediction
```

### RPC Service

launch server side
```
python -m paddle_serving_server.serve --model ResNet50_vd_model --port 9696 #cpu inference service
```

```
python -m paddle_serving_server.serve --model ResNet50_vd_model --port 9696 --gpu_ids 0 #gpu inference service
```

client send inference request
```
python resnet50_rpc_client.py ResNet50_vd_client_config/serving_client_conf.prototxt
```
*the port of server side in this example is 9696
