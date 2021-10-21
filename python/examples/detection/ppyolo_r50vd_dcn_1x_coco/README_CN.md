# 使用Paddle Serving部署PP-YOLO模型

(简体中文|[English](./README.md))

## 获得模型
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/ppyolo_r50vd_dcn_1x_coco.tar
```


### 启动服务
```
tar xf ppyolo_r50vd_dcn_1x_coco.tar
python3 -m paddle_serving_server.serve --model serving_server --port 9494 --gpu_ids 0
```

该模型支持TensorRT，如果想要更快的预测速度，可以开启`--use_trt`选项。

### 执行预测
```
python3 test_client.py 000000570688.jpg
```
