# Faster RCNN model on Pipeline Paddle Serving

### Get The Faster RCNN Model
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/faster_rcnn_r50_fpn_1x_coco.tar
```

### Start the service
```
tar xf faster_rcnn_r50_fpn_1x_coco.tar
python3 web_service.py
```

### Perform prediction

```
python3 pipeline_http_client.py
```
