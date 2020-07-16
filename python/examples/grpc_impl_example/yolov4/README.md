# Yolov4 Detection Service

([简体中文](README_CN.md)|English)

## Get Model

```
python -m paddle_serving_app.package --get_model yolov4
tar -xzvf yolov4.tar.gz
```

## Start RPC Service

```
python -m paddle_serving_server_gpu.serve --model yolov4_model --port 9393 --gpu_ids 0 --use_multilang
```

## Prediction

```
python test_client.py 000000570688.jpg
```
After the prediction is completed, a json file to save the prediction result and a picture with the detection result box will be generated in the `./outpu folder.
