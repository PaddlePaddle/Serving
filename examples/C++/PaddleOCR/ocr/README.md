# OCR 

(English|[简体中文](./README_CN.md))

## Get Model
```
python3 -m paddle_serving_app.package --get_model ocr_rec
tar -xzvf ocr_rec.tar.gz
python3 -m paddle_serving_app.package --get_model ocr_det
tar -xzvf ocr_det.tar.gz
```

## Get Dataset (Optional)
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/ocr/test_imgs.tar
tar xf test_imgs.tar
```

## Web Service

### Start Service

```
#choose one of cpu/gpu commands as following
#for cpu user
python3 -m paddle_serving_server.serve --model ocr_det_model --port 9293
python3 ocr_web_server.py cpu
#for gpu user
python3 -m paddle_serving_server.serve --model ocr_det_model --port 9293 --gpu_ids 0
python3 ocr_web_server.py gpu
```

### Client Prediction
```
python3 ocr_web_client.py
```
If you want a faster web service, please try Web LocalPredictor Service

## Web LocalPredictor Service
```
#choose one of cpu/gpu commands as following
#for cpu user
python3 ocr_debugger_server.py cpu
#for gpu user
python3 ocr_debugger_server.py gpu 
```

## Web LocalPredictor Client Prediction
```
python3 ocr_web_client.py
```

## Benchmark

CPU: Intel(R) Xeon(R) Gold 6148 CPU @ 2.40GHz * 40

GPU: Nvidia Tesla V100 * 1

Dataset: RCTW 500 sample images

| engine                       | client read image(ms) | client-server tras time(ms) | server read image（ms） | det pre(ms) | det infer(ms) | det post(ms) | rec pre(ms) | rec infer(ms) | rec post(ms) | server-client trans time(ms) | server side time consumption(ms) | server side overhead(ms) | total time（ms) |
|------------------------------|----------------|----------------------------|------------------|--------------------|------------------|--------------------|--------------------|------------------|--------------------|--------------------------|--------------------|--------------|---------------|
| Serving web service          | 8.69         | 13.41                      | 109.97           | 2.82               | 87.76            | 4.29               | 3.98               | 78.51            | 3.66               | 4.12                     | 181.02             | 136.49       | 317.51        |
| Serving LocalPredictor web service |  8.73        | 16.42                      | 115.27           | 2.93               | 20.63            | 3.97               | 4.48               | 13.84            | 3.60               | 6.91                     | 49.45              | 147.33       | 196.78        |

## Appendix: For Users who want to launch Det or Rec only
if you are going to detect images not recognize it or directly recognize the words from images. We also provide Det and Rec server for you.

### Det Server 

```
python3 det_web_server.py cpu #for cpu user
python3 det_web_server.py gpu #for gpu user
#or
python3 det_debugger_server.py cpu #for cpu user
python3 det_debugger_server.py gpu #for gpu user
```

### Det Client

```
# also use ocr_web_client.py
python3 ocr_web_client.py
```

### Rec Server

```
python3 rec_web_server.py cpu #for cpu user
python3 rec_web_server.py gpu #for gpu user
#or
python3 rec_debugger_server.py cpu #for cpu user
python3 rec_debugger_server.py gpu #for gpu user
```

### Rec Client

```
python3 rec_web_client.py
```

## C++ OCR Service

**Notice：** If you need to concatenate det model and rec model, and do pre-processing and post-processing in Paddle Serving C++ framework, you need to use the C++ server compiled with WITH_OPENCV option，see the [COMPILE.md](../../../../doc/Compile_EN.md)

### Start Service

When a service starts the concatenation of two models, it only needs to pass in the relative path of the model folder in order after `--model`, and the custom C++ OP class name after `--op`. The order of the model after `--model` and the class name after `--OP` needs to correspond. Here, it is assumed that we have defined the two OPs as GeneralDetectionOp and GeneralRecOp respectively, The script code is as follows:
```python
#One service starts the concatenation of two models
python3 -m paddle_serving_server.serve --model ocr_det_model ocr_rec_model --op GeneralDetectionOp GeneralInferOp --port 9293
#ocr_det_model correspond to GeneralDetectionOp, ocr_rec_model correspond to GeneralRecOp
```

### Client Prediction
The pre-processing and post-processing is in the C++ server part, the image's Base64 encoded string is passed into the C++ server.

so the value of parameter `feed_var` which is in the file `ocr_det_client/serving_client_conf.prototxt` should be changed.

for this case, `feed_type` should be 20(which means the data type is string),`shape` should be 1.

By passing in multiple client folder paths, the client can be started for multi model prediction.
```
python3 ocr_cpp_client.py ocr_det_client ocr_rec_client
```
