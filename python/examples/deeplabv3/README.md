# Image Segmentation

## Get Model

```
python3 -m paddle_serving_app.package --get_model deeplabv3
tar -xzvf deeplabv3.tar.gz
```

## RPC Service

### Start Service

```
python3 -m paddle_serving_server.serve --model deeplabv3_server --gpu_ids 0 --port 9494
```

### Client Prediction

```
python3 deeplabv3_client.py
```
