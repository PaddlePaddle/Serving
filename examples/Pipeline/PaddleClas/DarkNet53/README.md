# Imagenet Pipeline WebService

This document will takes Imagenet service as an example to introduce how to use Pipeline WebService.

## Get model
```
sh get_model.sh
```

## Start server

```
python3 resnet50_web_service.py &>log.txt &
```

## RPC test
```
python3 pipeline_rpc_client.py
```
