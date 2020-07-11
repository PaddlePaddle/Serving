# OCR 

## Get Model
```
python -m paddle_serving_app.package --get_model ocr_rec
tar -xzvf ocr_rec.tar.gz
python -m paddle_serving_app.package --get_model ocr_det
tar -xzvf ocr_det.tar.gz
```

## RPC Service

### Start Service

For the following two code block, please check your devices and pick one  
for GPU device
```
python -m paddle_serving_server_gpu.serve --model ocr_rec_model --port 9292 --gpu_id 0
python -m paddle_serving_server_gpu.serve --model ocr_det_model --port 9293 --gpu_id 0
```
for CPU device
```
python -m paddle_serving_server.serve --model ocr_rec_model --port 9292
python -m paddle_serving_server.serve --model ocr_det_model --port 9293
```

### Client Prediction

```
python ocr_rpc_client.py
```

## Web Service

### Start Service

```
python -m paddle_serving_server_gpu.serve --model ocr_det_model --port 9293 --gpu_id 0
python ocr_web_server.py
```

### Client Prediction
```
python ocr_web_client.py
```

## Web Debugger Service
```
python ocr_debugger_server.py
```

## Web Debugger Client Prediction
```
python ocr_web_clint.py
```
