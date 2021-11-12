# Image Classification

## Get Model

```
python3 -m paddle_serving_app.package --get_model mobilenet_v2_imagenet
tar -xzvf mobilenet_v2_imagenet.tar.gz
```

## RPC Service

### Start Service

```
python3 -m paddle_serving_server.serve --model mobilenet_v2_imagenet_model --gpu_ids 0 --port 9393
```

### Client Prediction

```
python3 mobilenet_tutorial.py
```
