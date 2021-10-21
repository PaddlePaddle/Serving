# 使用Paddle Serving部署Cascade RCNN模型

(简体中文|[English](./README.md))

## 获得Cascade RCNN模型
```
sh get_data.sh
```
如果你想要更多的检测模型，请参考[Paddle检测模型库](https://github.com/PaddlePaddle/PaddleDetection/blob/release/0.2/docs/MODEL_ZOO_cn.md)

### 启动服务
```
python3 -m paddle_serving_server.serve --model serving_server --port 9292 --gpu_id 0
```

### 执行预测
```
python3 test_client.py 000000570688.jpg
```

客户端已经为图片做好了后处理，在`output`文件夹下存放各个框的json格式信息还有后处理结果图片。
