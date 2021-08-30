# PP-YOLO model on Paddle Serving

([简体中文](./README_CN.md)|English)

### Get The Model
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/ppyolo_r50vd_dcn_1x_coco.tar
```

### Start the service
```
tar xf ppyolo_r50vd_dcn_1x_coco.tar
python3 -m paddle_serving_server.serve --model serving_server --port 9494 --gpu_ids 0
```

This model support TensorRT, if you want a faster inference, please use `--use_trt`.

### Perform prediction
```
python3 test_client.py 000000570688.jpg
```
