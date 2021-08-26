# Image Classification

## Get Model

```
python3 -m paddle_serving_app.package --get_model resnet_v2_50_imagenet
tar -xzvf resnet_v2_50_imagenet.tar.gz
```

## RPC Service

### Start Service

```
python3 -m paddle_serving_server.serve --model resnet_v2_50_imagenet_model --gpu_ids 0 --port 9393
```

### Client Prediction

```
python3 resnet50_v2_tutorial.py
```
