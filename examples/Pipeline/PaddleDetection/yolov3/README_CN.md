# YOLOv3 model on Pipeline Paddle Serving

(简体中文|[English](./README.md))

### 获取模型
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/yolov3_darknet53_270e_coco.tar
```

### 启动 WebService
```
tar xf yolov3_darknet53_270e_coco.tar
python3 web_service.py
```

### 执行预测
```
python3 pipeline_http_client.py
```
