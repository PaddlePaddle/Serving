# OCR Pipeline WebService 

(English|[简体中文](./README_CN.md))

This document will take OCR as an example to show how to use Pipeline WebService to start multi-model tandem services.

This OCR example only supports Process OP.

## Get Model
```
python3 -m paddle_serving_app.package --get_model ocr_rec
tar -xzvf ocr_rec.tar.gz
python3 -m paddle_serving_app.package --get_model ocr_det
tar -xzvf ocr_det.tar.gz
```

## Get Dataset (Optional)
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/ocr/test_imgs.tar
tar xf test_imgs.tar
```

## Run services

### 1.Start a single server and client.
```
python3 web_service.py &>log.txt &
```

Test
```
python3 pipeline_http_client.py
```

<!--
## More (PipelineServing)

## Client Prediction

### RPC
```
python3 pipeline_rpc_client.py
```

### HTTP
```
python3 pipeline_http_client.py
```
-->


### 2.Run benchmark
```
python3 web_service.py &>log.txt &
```

Test
```
sh benchmark.sh
```
