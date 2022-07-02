# OCR Pipeline WebService

([English](./README.md)|简体中文)

本文档将以 OCR 为例，介绍如何使用 Pipeline WebService 启动多模型串联的服务。
本示例仅支持进程OP模式。

## 获取模型
```
python3 -m paddle_serving_app.package --get_model ocr_rec
tar -xzvf ocr_rec.tar.gz
python3 -m paddle_serving_app.package --get_model ocr_det
tar -xzvf ocr_det.tar.gz
```

## 获取数据集（可选）
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/ocr/test_imgs.tar
tar xf test_imgs.tar
```

## 启动 WebService

### 1.启动单server、单client
```
python3 web_service.py &>log.txt &
```

## 测试
```
python3 pipeline_http_client.py
```

<!--
## 其他 (PipelineServing)

## 启动客户端

### RPC

```
python3 pipeline_rpc_client.py
```

### HTTP

```
python3 pipeline_http_client.py
```
-->

### 2.启动 benchmark
```
python3 web_service.py &>log.txt &
```

Test
```
sh benchmark.sh
```
