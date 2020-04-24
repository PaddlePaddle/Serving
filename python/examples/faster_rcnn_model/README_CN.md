# Faster RCNN模型

(简体中文|[English](./README.md))

本文需要[Paddle Detection](https://github.com/PaddlePaddle/PaddleDetection)训练的模型和配置文件。如果用户想要快速部署在Paddle Serving上，请直接阅读第二章节。

## 1. 训练物体检测模型

用户可以阅读 [Paddle Detection入门使用](https://github.com/PaddlePaddle/PaddleDetection/blob/release/0.2/docs/tutorials/GETTING_STARTED_cn.md)来了解Paddle Detection的背景。PaddleDetection的目的是为工业界和学术界提供丰富、易用的目标检测模型。不仅性能优越、易于部署，而且能够灵活的满足算法研究的需求。

### 环境要求

CPU版： 没有特别要求

GPU版： CUDA 9.0及以上

```
git clone https://github.com/PaddlePaddle/PaddleDetection
cd PaddleDetection
```
接下来可以训练faster rcnn模型
```
python tools/train.py -c configs/faster_rcnn_r50_1x.yml
```
训练模型的时间视情况而定，与训练的设备算力和迭代轮数相关。
在训练的过程中，`faster_rcnn_r50_1x.yml`当中定义了保存模型的`snapshot`，在最终训练完成后，效果最好的模型，会被保存为`best_model.pdmodel`，这是一个经过压缩的PaddleDetection的专属模型文件。

**如果我们要让模型可被Paddle Serving所使用，必须做export_model。**

输出模型
```
python tools/export_model.py -c configs/faster_rcnn_r50_1x.yml \
        --output_dir=./inference_model \
        -o weights=output/faster_rcnn_r50_1x/model_final 
```
预测模型会导出到`inference_model/faster_rcnn_r50_1x`目录下，模型名和参数名分别为`__model__`和`__params__`。

接下来我们请参考 [如何从Paddle保存的预测模型转为Paddle Serving格式可部署的模型](https://github.com/PaddlePaddle/Serving/blob/develop/doc/INFERNCE_TO_SERVING_CN.md)转换成Serving可用的模型。其中`serving_server_dir`赋值为`pddet_serving_model`,`serving_client_dir`赋值为`pddet_client_conf`。


## 2. 启动模型并预测
如果用户没有用Paddle Detection项目训练模型，我们也在此为您提供示例模型下载。如果您用Paddle Detection训练了模型，可以跳过 **下载模型** 部分。

### 下载模型
```
wget https://paddle-serving.bj.bcebos.com/pddet_demo/faster_rcnn_model.tar.gz
wget https://paddle-serving.bj.bcebos.com/pddet_demo/infer_cfg.yml
tar xf faster_rcnn_model.tar.gz
mv faster_rcnn_model/pddet* .
```

### 启动服务
```
GLOG_v=2 python -m paddle_serving_server_gpu.serve --model pddet_serving_model --port 9494 --gpu_id 0
```

### 执行预测
```
python test_client.py pddet_client_conf/serving_client_conf.prototxt infer_cfg.yml 000000570688.jpg 
```

## 3. 结果分析
<p align="center">
    <br>
<img src='000000570688.jpg' >
    <br>
<p>
这是输入图片
  
<p align="center">
    <br>
<img src='000000570688_bbox.jpg' >
    <br>
<p>
这是实现添加了bbox之后的图片，可以看到客户端已经为图片做好了后处理，此外在output/bbox.json也有各个框的编号和坐标信息。
