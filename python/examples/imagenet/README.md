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

### HTTP Infer

launch server side
```
python image_classification_service.py ResNet50_vd_model workdir 9393 #cpu inference service
```
```
python image_classification_service_gpu.py ResNet50_vd_model workdir 9393 #gpu inference service
```


client send inference request
```
python image_http_client.py
```
### RPC Infer

launch server side
```
python -m paddle_serving_server.serve --model ResNet50_vd_model --port 9393 #cpu inference service
```

```
python -m paddle_serving_server_gpu.serve --model ResNet50_vd_model --port 9393 --gpu_ids 0 #gpu inference service
```

client send inference request
```
python image_rpc_client.py ResNet50_vd_client_config/serving_client_conf.prototxt
```
*the port of server side in this example is 9393, the sample data used by client side is in the folder ./data. These parameter can be modified in practice*
