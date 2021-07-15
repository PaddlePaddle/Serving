# Pyramidbox_lite_server_mask

## Get Model
```
hub install pyramidbox_lite_server_mask
hub install pyramidbox_lite_server
model in /home/user/.paddlehub/
copy model to our project,then export model to serving model.
```

## RPC Service

### Start Service

```
# python -m paddle_serving_server.serve --model face_serving_server --port 9494
# python -m paddle_serving_server.serve --model mask_serving_server --port 9495
```

### Client Prediction

```
python test_client.py mask_serving_client/serving_client_conf.prototxt test.jpg
```

the result is in `output` folder, including a json file and image file with bounding boxes.
