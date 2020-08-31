# OCR Pipeline WebService 

(English|[简体中文](./README_CN.md))

This document will take OCR as an example to show how to use Pipeline WebService to start multi-model tandem services.

## Get Model
```
python -m paddle_serving_app.package --get_model ocr_rec
tar -xzvf ocr_rec.tar.gz
python -m paddle_serving_app.package --get_model ocr_det
tar -xzvf ocr_det.tar.gz
```

## Get Dataset (Optional)
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/ocr/test_imgs.tar
tar xf test_imgs.tar
```

## Start Service
```
python web_service.py &>log.txt &
```

## Test
```
python pipeline_http_client.py
```



<!--
## More (PipelineServing)

You can choose one of the following versions to start Service.

### Remote Service Version
```
python -m paddle_serving_server_gpu.serve --model ocr_det_model --port 12000 --gpu_id 0 &> det.log &
python -m paddle_serving_server_gpu.serve --model ocr_rec_model --port 12001 --gpu_id 0 &> rec.log &
python remote_service_pipeline_server.py &>pipeline.log &
```

### Local Service Version
```
python local_service_pipeline_server.py &>pipeline.log &
```

### Hybrid Service Version
```
python -m paddle_serving_server_gpu.serve --model ocr_rec_model --port 12001 --gpu_id 0 &> rec.log &
python hybrid_service_pipeline_server.py &>pipeline.log &
```

## Client Prediction

### RPC
```
python pipeline_rpc_client.py
```

### HTTP
```
python pipeline_http_client.py
```
-->
