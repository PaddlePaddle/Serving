# Faster RCNN model on Paddle Serving

([简体中文](./README_CN.md)|English)

### Get The Faster RCNN Model
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/pddet_demo/faster_rcnn_model.tar.gz
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/pddet_demo/infer_cfg.yml
```
If you want to have more detection models, please refer to [Paddle Detection Model Zoo](https://github.com/PaddlePaddle/PaddleDetection/blob/release/0.2/docs/MODEL_ZOO_cn.md)

### Start the service
```
tar xf faster_rcnn_model.tar.gz
mv faster_rcnn_model/pddet* .
GLOG_v=2 python -m paddle_serving_server_gpu.serve --model pddet_serving_model --port 9494 --gpu_ids 0
```

### Perform prediction
```
python test_client.py pddet_client_conf/serving_client_conf.prototxt infer_cfg.yml 000000570688.jpg
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
