# OCR 

(English|[简体中文](./README_CN.md))

## Get Model
```
python -m paddle_serving_app.package --get_model ocr_rec
tar -xzvf ocr_rec.tar.gz
python -m paddle_serving_app.package --get_model ocr_det
tar -xzvf ocr_det.tar.gz
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
If you want a faster web service, please try Web Debugger Service

## Web Debugger Service
```
python ocr_debugger_server.py
```

## Web Debugger Client Prediction
```
python ocr_web_clint.py
```

## Benchmark

CPU: Intel(R) Xeon(R) Gold 6148 CPU @ 2.40GHz * 40

GPU: Nvidia Tesla V100 * 1

| engine                       | client read image(ms) | client-server tras time(ms) | server read image（ms） | det pre(ms) | det infer(ms) | det post(ms) | rec pre(ms) | rec infer(ms) | rec post(ms) | server-client trans time(ms) | server side time consumption(ms) | server side overhead(ms) | total time（ms) |
|------------------------------|----------------|----------------------------|------------------|--------------------|------------------|--------------------|--------------------|------------------|--------------------|--------------------------|--------------------|--------------|---------------|
| Serving web service          | 112.02         | 13.41                      | 109.97           | 2.82               | 87.76            | 4.29               | 3.98               | 78.51            | 3.66               | 4.12                     | 181.02             | 239.52       | 420.54        |
| Serving Debugger web service | 111.68         | 16.42                      | 115.27           | 2.93               | 20.63            | 3.97               | 4.48               | 13.84            | 3.60               | 6.91                     | 49.45              | 250.28       | 299.73        |
