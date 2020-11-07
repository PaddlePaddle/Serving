# OCR 

(English|[简体中文](./README_CN.md))

## Get Model
```
python -m paddle_serving_app.package --get_model ocr_rec
tar -xzvf ocr_rec.tar.gz
python -m paddle_serving_app.package --get_model ocr_det
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
python -m paddle_serving_server.serve --model ocr_det_model --port 9293
python ocr_web_server.py cpu
#for gpu user
python -m paddle_serving_server_gpu.serve --model ocr_det_model --port 9293 --gpu_id 0
python ocr_web_server.py gpu
```

### Client Prediction
```
python ocr_web_client.py
```
If you want a faster web service, please try Web LocalPredictor Service

## Web LocalPredictor Service
```
#choose one of cpu/gpu commands as following
#for cpu user
python ocr_debugger_server.py cpu
#for gpu user
python ocr_debugger_server.py gpu 
```

## Web LocalPredictor Client Prediction
```
python ocr_web_client.py
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
python det_web_server.py cpu #for cpu user
python det_web_server.py gpu #for gpu user
#or
python det_debugger_server.py cpu #for cpu user
python det_debugger_server.py gpu #for gpu user
```

### Det Client

```
# also use ocr_web_client.py
python ocr_web_client.py
```

### Rec Server

```
python rec_web_server.py cpu #for cpu user
python rec_web_server.py gpu #for gpu user
#or
python rec_debugger_server.py cpu #for cpu user
python rec_debugger_server.py gpu #for gpu user
```

### Rec Client

```
python rec_web_client.py
```
