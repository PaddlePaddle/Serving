# Image Segmentation

## Get Model

```
python3 -m paddle_serving_app.package --get_model unet
tar -xzvf unet.tar.gz
```

## RPC Service

### Start Service

```
python3 -m paddle_serving_server.serve --model unet_model --gpu_ids 0 --port 9494
```

### Client Prediction

```
python3 seg_client.py
```
