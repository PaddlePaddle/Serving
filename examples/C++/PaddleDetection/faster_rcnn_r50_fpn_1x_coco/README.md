# Faster RCNN model on Paddle Serving

([简体中文](./README_CN.md)|English)

### Get The Faster RCNN Model
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/faster_rcnn_r50_fpn_1x_coco.tar
```

### Start the service
```
tar xf faster_rcnn_r50_fpn_1x_coco.tar
python3 -m paddle_serving_server.serve --model serving_server --port 9494 --gpu_ids 0
```

This model support TensorRT, if you want a faster inference, please use `--use_trt`. But you need to do some extra work.
Please reference to https://github.com/PaddlePaddle/Paddle-Inference-Demo/blob/master/c%2B%2B/paddle-trt/trt_dynamic_shape_test.cc#L40 


### Perform prediction
```
python3 test_client.py 000000570688.jpg
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
