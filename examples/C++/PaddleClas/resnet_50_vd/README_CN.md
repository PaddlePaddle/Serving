# 图像分类

## 1.获取模型

```
wget https://paddle-imagenet-models-name.bj.bcebos.com/dygraph/inference/ResNet50_vd_infer.tar && tar xf ResNet50_vd_infer.tar

```

## 2.用 paddle_serving_client 把下载的推理模型保存用于 Serving 部署的模型参数
```
# 保存 ResNet50_vd 模型参数
python3 -m paddle_serving_client.convert --dirname ./ResNet50_vd_infer/ \
                                         --model_filename inference.pdmodel  \
                                         --params_filename inference.pdiparams \
                                         --serving_server ./ResNet50_vd_serving/ \
                                         --serving_client ./ResNet50_vd_client/
```
会在当前文件夹多出 `ResNet50_vd_serving` 和 `ResNet50_vd_client` 的文件夹


保存参数后，会在当前文件夹多出 `ResNet50_vd_serving` 和 `ResNet50_vd_client` 的文件夹：
```
├── daisy.jpg
├── http_client.py
├── imagenet.label
├── ResNet50_vd_client
│   ├── serving_client_conf.prototxt
│   └── serving_client_conf.stream.prototxt
├── ResNet50_vd_infer
│   ├── inference.pdiparams
│   ├── inference.pdiparams.info
│   └── inference.pdmodel
├── ResNet50_vd_serving
│   ├── fluid_time_file
│   ├── inference.pdiparams
│   ├── inference.pdmodel
│   ├── serving_server_conf.prototxt
│   └── serving_server_conf.stream.prototxt
├── rpc_client.py
```

**三.启动服务**

C++ Serving 服务可以指定一个网络端口同时接收 HTTP、gRPC 和 bRPC 请求。命令参数 `--model` 指定模型路径，`--gpu_ids` 指定 GPU 卡，`--port` 指定端口。

```
python3 -m paddle_serving_server.serve --model ResNet50_vd_serving --gpu_ids 0 --port 9394
```

**四.启动客户端**

1. `rpc_client.py` 封装了 HTTP 请求客户端

```
python3 http_client.py
```

2. `http_client.py` 封装了 gRPC 请求客户端

```
python3 rpc_client.py
```

成功运行后，模型预测的结果会打印如下：

```
prediction: daisy, probability: 0.9341399073600769
```
