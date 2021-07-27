# Faster RCNN HRNet model on Paddle Serving

([简体中文](./README_CN.md)|English)

### Get The Faster RCNN HRNet Model
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/faster_rcnn_hrnetv2p_w18_1x.tar
```

### Start the service
```
tar xf faster_rcnn_hrnetv2p_w18_1x.tar
python -m paddle_serving_server.serve --model serving_server --port 9494 --gpu_ids 0
```

This model support TensorRT, if you want a faster inference, please use `--use_trt`. 


### Prediction
```
python test_client.py 000000570688.jpg
```
