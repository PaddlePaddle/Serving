# 使用Paddle Serving部署YOLOv3模型

(简体中文|[English](./README.md))

## 获得模型
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/yolov3_darknet53_270e_coco.tar
```


### 启动服务
```
tar xf yolov3_darknet53_270e_coco.tar
python3 -m paddle_serving_server.serve --model serving_server --port 9494 --gpu_ids 0
```

该模型支持TensorRT，如果想要更快的预测速度，可以开启`--use_trt`选项。

### 执行预测
```
python3 test_client.py 000000570688.jpg
```
