# 使用Paddle Serving部署SSD模型

(简体中文|[English](./README.md))

## 获得模型
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/ssd_vgg16_300_240e_voc.tar
```


### 启动服务
```
tar xf ssd_vgg16_300_240e_voc.tar
python3 -m paddle_serving_server.serve --model serving_server --port 9494 --gpu_ids 0
```

该模型支持TensorRT，如果想要更快的预测速度，可以开启`--use_trt`选项。

### 执行预测
```
python3 test_client.py 000000014439.jpg
```
