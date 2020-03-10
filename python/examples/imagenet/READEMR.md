# Image Classification Service

model : ResNet50_vd


## Prepare model

```
sh get_model.sh
```

## Start web service
```
python image_classification_service.py serving_server_model/ workdir/ 9393
```

## Prediction 
```
python image_http_client.py
```

## Start rpc service

```
python image_server.py serving_server_model/ 9393
```

## Prediction
```
python image_rpc_client.py serving_client_conf/serving_client_conf.prototxt
```
