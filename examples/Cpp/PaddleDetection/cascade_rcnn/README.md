# Cascade RCNN model on Paddle Serving

([简体中文](./README_CN.md)|English)

### Get The Cascade RCNN Model
```
sh get_data.sh
```
If you want to have more detection models, please refer to [Paddle Detection Model Zoo](https://github.com/PaddlePaddle/PaddleDetection/blob/release/0.2/docs/MODEL_ZOO_cn.md)

### Start the service
```
python3 -m paddle_serving_server.serve --model serving_server --port 9292 --gpu_id 0
```

### Perform prediction
```
python3 test_client.py 000000570688.jpg
```

Image with bounding boxes and json result would be saved in `output` folder.
