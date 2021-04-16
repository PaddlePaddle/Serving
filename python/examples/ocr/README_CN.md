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

## Web Service服务

### 启动服务

```
#根据CPU/GPU设备选择一种启动方式
#for cpu user
python -m paddle_serving_server.serve --model ocr_det_model --port 9293
python ocr_web_server.py cpu
#for gpu user
python -m paddle_serving_server.serve --model ocr_det_model --port 9293 --gpu_id 0
python ocr_web_server.py gpu
```

### 启动客户端
```
python ocr_web_client.py
```

如果用户需要更快的执行速度，请尝试LocalPredictor版Web服务
## 启动LocalPredictor版Web服务
```
#根据CPU/GPU设备选择一种启动方式
#for cpu user
python ocr_debugger_server.py cpu
#for gpu user
python ocr_debugger_server.py gpu
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
| Serving LocalPredictor web service | 8.73         | 16.42                      | 115.27           | 2.93               | 20.63            | 3.97               | 4.48               | 13.84            | 3.60               | 6.91                     | 49.45              | 147.33      | 196.78        |


## 附录： 检测/识别单服务启动
如果您想单独启动检测或者识别服务，我们也提供了启动单服务的代码

### 启动检测服务

```
python det_web_server.py cpu #for cpu user
python det_web_server.py gpu #for gpu user
#or
python det_debugger_server.py cpu #for cpu user
python det_debugger_server.py gpu #for gpu user
```

### 检测服务客户端

```
# also use ocr_web_client.py
python ocr_web_client.py
```

### 启动识别服务

```
python rec_web_server.py cpu #for cpu user
python rec_web_server.py gpu #for gpu user
#or
python rec_debugger_server.py cpu #for cpu user
python rec_debugger_server.py gpu #for gpu user
```

### 识别服务客户端

```
python rec_web_client.py
```
## C++ OCR Service服务

**注意：** 若您需要使用Paddle Serving C++框架串联det模型和rec模型，并进行前后处理，您需要使用开启WITH_OPENCV选项编译的C++ Server，详见[COMPILE.md](../../../doc/COMPILE.md)

### 启动服务
根据CPU/GPU设备选择一种启动方式

通过--model后，指定多个模型文件的文件夹路径来启动多模型串联的预测服务。
```
#for cpu user
python -m paddle_serving_server.serve --model ocr_det_model ocr_rec_model --port 9293
#for gpu user
python -m paddle_serving_server.serve --model ocr_det_model ocr_rec_model --port 9293 --gpu_id 0
```

### 启动客户端
由于需要在C++Server部分进行前后处理，传入C++Server的仅仅是图片的base64编码的字符串，故第一个模型的Client配置需要修改

即`ocr_det_client/serving_client_conf.prototxt`中`feed_var`字段

对于本示例而言，`feed_type`应修改为3(数据类型为string),`shape`为1.

通过在客户端启动后加入多个client模型的client配置文件夹路径，启动client进行预测。
```
python ocr_cpp_client.py ocr_det_client ocr_rec_client
```
