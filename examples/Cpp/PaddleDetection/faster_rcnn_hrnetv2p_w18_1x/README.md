# Faster RCNN HRNet model on Paddle Serving

([简体中文](./README_CN.md)|English)

### Get The Faster RCNN HRNet Model
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/pddet_demo/faster_rcnn_hrnetv2p_w18_1x.tar.gz
```

### Start the service
```
tar xf faster_rcnn_hrnetv2p_w18_1x.tar.gz
python3 -m paddle_serving_server.serve --model serving_server --port 9494 --gpu_ids 0
```

This model support TensorRT, if you want a faster inference, please use `--use_trt`. But you need to do some extra work.
Please reference to https://github.com/PaddlePaddle/Paddle-Inference-Demo/blob/master/c%2B%2B/paddle-trt/trt_dynamic_shape_test.cc#L40 


### Prediction
```
python3 test_client.py 000000570688.jpg
```
