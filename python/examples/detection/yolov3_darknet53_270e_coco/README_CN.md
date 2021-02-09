# 使用Paddle Serving部署YOLOv3模型

(简体中文|[English](./README.md))

## 获得模型
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/yolov3_darknet53_270e_coco.tar
```


### 启动服务
```
tar xf yolov3_darknet53_270e_coco.tar
python -m paddle_serving_server_gpu.serve --model pddet_serving_model --port 9494 --gpu_ids 0
```

### 执行预测
```
python test_client.py 000000570688.jpg
```

