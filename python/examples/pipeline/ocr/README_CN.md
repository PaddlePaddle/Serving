# OCR Pipeline WebService

([English](./README.md)|简体中文)

本文档将以 OCR 为例，介绍如何使用 Pipeline WebService 启动多模型串联的服务。

## 获取模型
```
python -m paddle_serving_app.package --get_model ocr_rec
tar -xzvf ocr_rec.tar.gz
python -m paddle_serving_app.package --get_model ocr_det
tar -xzvf ocr_det.tar.gz
```

## 获取数据集（可选）
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/ocr/test_imgs.tar
tar xf test_imgs.tar
```

## 启动 WebService
```
python web_service.py &>log.txt &
```

## 测试
```
python pipeline_http_client.py
```

<!--
## 其他 (PipelineServing)

你可以选择下面任意一种版本启动服务。

### 远程服务版本
```
python -m paddle_serving_server.serve --model ocr_det_model --port 12000 --gpu_id 0 &> det.log &
python -m paddle_serving_server.serve --model ocr_rec_model --port 12001 --gpu_id 0 &> rec.log &
python remote_service_pipeline_server.py &>pipeline.log &
```

### 本地服务版本
```
python local_service_pipeline_server.py &>pipeline.log &
```

### 混合服务版本
```
python -m paddle_serving_server_gpu.serve --model ocr_rec_model --port 12001 --gpu_id 0 &> rec.log &
python hybrid_service_pipeline_server.py &>pipeline.log &
```

## 启动客户端

### RPC

```
python pipeline_rpc_client.py
```

### HTTP

```
python pipeline_http_client.py
```
-->
