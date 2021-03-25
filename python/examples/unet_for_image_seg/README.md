# Image Segmentation

## Get Model

```
python -m paddle_serving_app.package --get_model unet
tar -xzvf unet.tar.gz
```

## RPC Service

### Start Service

```
python -m paddle_serving_server.serve --model unet_model --gpu_ids 0 --port 9494
```

### Client Prediction

```
python seg_client.py
```
