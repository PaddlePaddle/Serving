## Image Classification

([简体中文](./README_CN.md)|English)

The example uses the ResNet50_vd model to perform the imagenet 1000 classification task.

### Get model config and sample dataset
```
sh get_model.sh
```

### Install preprocess module

```
pip3 install paddle_serving_app
```


### Inference Service(Support BRPC-Client/GRPC-Client/Http-Client)

launch server side
```
python3 -m paddle_serving_server.serve --model ResNet50_vd_model --port 9696 #cpu inference service
```

```
python3 -m paddle_serving_server.serve --model ResNet50_vd_model --port 9696 --gpu_ids 0 #gpu inference service
```

### BRPC-Client
client send inference request
```
python3 resnet50_rpc_client.py ResNet50_vd_client_config/serving_client_conf.prototxt
```
*the port of server side in this example is 9696

### GRPC-Client/Http-Client
client send inference request
```
python3 resnet50_http_client.py ResNet50_vd_client_config/serving_client_conf.prototxt
```
