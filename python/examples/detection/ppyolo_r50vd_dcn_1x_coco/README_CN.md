# 使用Paddle Serving部署PP-YOLO模型

(简体中文|[English](./README.md))

## 获得模型
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/ppyolo_r50vd_dcn_1x_coco.tar
```


### 启动服务
```
tar xf ppyolo_r50vd_dcn_1x_coco.tar
python -m paddle_serving_server_gpu.serve --model pddet_serving_model --port 9494 --gpu_ids 0
```

### 执行预测
```
python test_client.py 000000570688.jpg
```

