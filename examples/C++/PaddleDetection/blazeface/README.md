# Blazeface 

## Get Model
```
python3 -m paddle_serving_app.package --get_model blazeface
tar -xf blazeface.tar.gz
```

## RPC Service

### Start Service

```
python3 -m paddle_serving_server.serve --model serving_server --port 9494
```

### Client Prediction

```
python3 test_client.py serving_client/serving_client_conf.prototxt test.jpg
```

the result is in `output` folder, including a json file and image file with bounding boxes.
