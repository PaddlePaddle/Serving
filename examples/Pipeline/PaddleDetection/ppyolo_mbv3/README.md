# PPYOLO model on Pipeline Paddle Serving

([简体中文](./README_CN.md)|English)

### Get Model
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/ppyolo_mbv3_large_coco.tar
```

### Start the service
```
tar xf ppyolo_mbv3_large_coco.tar
python3 web_service.py
```

### Perform prediction
```
python3 pipeline_http_client.py
```
