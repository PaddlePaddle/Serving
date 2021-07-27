# Imagenet Pipeline WebService

This document will takes Imagenet service as an example to introduce how to use Pipeline WebService.

## Get model
```
python -m paddle_serving_app.package --get_model resnet_v2_50_imagenet
tar -xzvf resnet_v2_50_imagenet.tar.gz
```

## Start server

```
python resnet50_web_service.py &>log.txt &
```

## RPC test
```
python pipeline_rpc_client.py
```
