# Imagenet Pipeline WebService

This document will takes Imagenet service as an example to introduce how to use Pipeline WebService.

## Get model
```
sh get_model.sh
python encrypt.py
```

## Start server

```
python -m paddle_serving_server.serve --model encrypt_server/ --port 9400  --encryption_rpc_port 9401  --use_encryption_model &
python web_service.py &>log.txt &
```

## client test
```
python http_client.py
```

if you configure the api gateway, you can use `https_client.py`
