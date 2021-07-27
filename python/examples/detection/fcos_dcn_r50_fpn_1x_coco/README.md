# FCOS model on Paddle Serving

([简体中文](./README_CN.md)|English)

### Get Model
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/fcos_dcn_r50_fpn_1x_coco.tar
```

### Start the service
```
tar xf fcos_dcn_r50_fpn_1x_coco.tar
python -m paddle_serving_server.serve --model serving_server --port 9494 --gpu_ids 0
```
This model support TensorRT, if you want a faster inference, please use `--use_trt`.

### Perform prediction
```
python test_client.py 000000570688.jpg
```
