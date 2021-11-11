# PPYOLO model on Pipeline Paddle Serving

(简体中文|[English](./README_CN.md))

### 获取模型
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/ppyolo_mbv3_large_coco.tar
```

### 启动服务
```
tar xf ppyolo_mbv3_large_coco.tar
python3 web_service.py
```

### 执行预测
```
python3 pipeline_http_client.py
```
