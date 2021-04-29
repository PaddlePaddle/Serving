# 使用Paddle Serving部署FCOS模型

(简体中文|[English](./README.md))

## 获得模型
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/fcos_dcn_r50_fpn_1x_coco.tar
```


### 启动服务
```
tar xf fcos_dcn_r50_fpn_1x_coco.tar
python -m paddle_serving_server.serve --model serving_server --port 9494 --gpu_ids 0
```

该模型支持TensorRT，如果想要更快的预测速度，可以开启`--use_trt`选项。

### 执行预测
```
python test_client.py 000000570688.jpg
```
