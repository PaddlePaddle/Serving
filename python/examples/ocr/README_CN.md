# OCR 服务

([English](./README.md)|简体中文)

## 获取模型
```
python -m paddle_serving_app.package --get_model ocr_rec
tar -xzvf ocr_rec.tar.gz
python -m paddle_serving_app.package --get_model ocr_det
tar -xzvf ocr_det.tar.gz
```
## 获取数据集（可选）
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/ocr/test_imgs.tar
tar xf test_imgs.tar
```

### 客户端预测

```
python ocr_rpc_client.py
```

## Web Service服务

### 启动服务

```
python -m paddle_serving_server_gpu.serve --model ocr_det_model --port 9293 --gpu_id 0
python ocr_web_server.py
```

### 启动客户端
```
python ocr_web_client.py
```

如果用户需要更快的执行速度，请尝试Debugger版Web服务
## 启动Debugger版Web服务
```
python ocr_debugger_server.py
```

## 启动客户端
```
python ocr_web_client.py
```

## 性能指标

CPU: Intel(R) Xeon(R) Gold 6148 CPU @ 2.40GHz * 40

GPU: Nvidia Tesla V100单卡

数据集：RCTW 500张测试数据集

| engine                       | 客户端读图(ms) | 客户端发送请求到服务端(ms) | 服务端读图（ms） | 检测预处理耗时(ms) | 检测模型耗时(ms) | 检测后处理耗时(ms) | 识别预处理耗时(ms) | 识别模型耗时(ms) | 识别后处理耗时(ms) | 服务端回传客户端时间(ms) | 服务端整体耗时(ms) | 空跑耗时(ms) | 整体耗时（ms) |
|------------------------------|----------------|----------------------------|------------------|--------------------|------------------|--------------------|--------------------|------------------|--------------------|--------------------------|--------------------|--------------|---------------|
| Serving web service          | 8.69         | 13.41                      | 109.97           | 2.82               | 87.76            | 4.29               | 3.98               | 78.51            | 3.66               | 4.12                     | 181.02             | 136.49      | 317.51        |
| Serving Debugger web service | 8.73         | 16.42                      | 115.27           | 2.93               | 20.63            | 3.97               | 4.48               | 13.84            | 3.60               | 6.91                     | 49.45              | 147.33      | 196.78        |


## 附录： 检测/识别单服务启动
如果您想单独启动检测或者识别服务，我们也提供了启动单服务的代码

### 启动检测服务

```
python det_web_server.py 
#or
python det_debugger_server.py
```

### 检测服务客户端

```
# also use ocr_web_client.py
python ocr_web_client.py
```

### 启动识别服务

```
python rec_web_server.py 
#or
python rec_debugger_server.py
```

### 识别服务客户端

```
python rec_web_client.py
```
