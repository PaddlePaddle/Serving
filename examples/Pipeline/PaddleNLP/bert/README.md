# Bert Pipeline WebService

This document will takes Bert service as an example to introduce how to use Pipeline WebService.

## Get model
```
sh get_data.sh
```

## Start server

```
python3 web_service.py &>log.txt &
```

## RPC test
```
python3 pipeline_rpc_client.py
```
