# Faster RCNN model on Paddle Serving

([简体中文](./README_CN.md)|English)

This article requires [Paddle Detection](https://github.com/PaddlePaddle/PaddleDetection) trained models and configuration files. If users want to quickly deploy on Paddle Serving, please read the Chapter 2 directly.

## 1. Train an object detection model

Users can read [Paddle Detection Getting Started](https://github.com/PaddlePaddle/PaddleDetection/blob/release/0.2/docs/tutorials/GETTING_STARTED_cn.md) to understand the background of Paddle Detection. The purpose of PaddleDetection is to provide a rich and easy-to-use object detection model for industry and academia. Not only is it superior in performance and easy to deploy, but it can also flexibly meet the needs of algorithm research.

### Environmental requirements

CPU version: No special requirements

GPU version: CUDA 9.0 and above

```
git clone https://github.com/PaddlePaddle/PaddleDetection
cd PaddleDetection
```
Next, you can train the faster rcnn model
```
python tools/train.py -c configs/faster_rcnn_r50_1x.yml
```
The time for training the model depends on the situation and is related to the computing power of the training equipment and the number of iterations.
In the training process, `faster_rcnn_r50_1x.yml` defines the snapshot of the saved model. After the final training, the model with the best effect will be saved as `best_model.pdmodel`, which is a compressed PaddleDetection Exclusive model files.

**If we want the model to be used by Paddle Serving, we must do export_model.**

Output model
```
python export_model.py
```
## 2. Start the model and predict
If users do not use the Paddle Detection project to train models, we are here to provide you with sample model downloads. If you trained the model with Paddle Detection, you can skip the ** Download Model ** section.

### Download model
```
wget https://paddle-serving.bj.bcebos.com/pddet_demo/faster_rcnn_model.tar.gz
wget https://paddle-serving.bj.bcebos.com/pddet_demo/paddle_serving_app-0.0.1-py2-none-any.whl
wget https://paddle-serving.bj.bcebos.com/pddet_demo/infer_cfg.yml
tar xf faster_rcnn_model.tar.gz
mv faster_rcnn_model/pddet *.
```

### Start the service
```
GLOG_v = 2 python -m paddle_serving_server_gpu.serve --model pddet_serving_model --port 9494 --gpu_id 0
```

### Perform prediction
```
python test_client.py --config_path = infer_cfg.yml --infer_img = 000000570688.jpg --dump_result --visualize
```

## 3. Result analysis
<p align = "center">
    <br>
<img src = '000000570688.jpg'>
    <br>
<p>
This is the input picture
  
<p align = "center">
    <br>
<img src = '000000570688_bbox.jpg'>
    <br>
<p>
  
This is the picture after adding bbox. You can see that the client has done post-processing for the picture. In addition, the output/bbox.json also has the number and coordinate information of each box.



