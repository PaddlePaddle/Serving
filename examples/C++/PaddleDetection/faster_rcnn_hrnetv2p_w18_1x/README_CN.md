# 使用Paddle Serving部署Faster RCNN HRNet模型

(简体中文|[English](./README.md))

## 获得Faster RCNN HRNet模型
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/pddet_demo/faster_rcnn_hrnetv2p_w18_1x.tar.gz
```


### 启动服务
```
tar xf faster_rcnn_hrnetv2p_w18_1x.tar.gz
python3 -m paddle_serving_server.serve --model serving_server --port 9494 --gpu_ids 0
```
该模型支持TensorRT，如果想要更快的预测速度，可以开启`--use_trt`选项,但此时需要额外设置子图的TRT变长最大最小最优shape.
请参考https://github.com/PaddlePaddle/Paddle-Inference-Demo/blob/master/c%2B%2B/paddle-trt/trt_dynamic_shape_test.cc#L40

### 执行预测
```
python3 test_client.py 000000570688.jpg
```
