# resnet50 int8 example
(English|[简体中文](./README_CN.md))

## Obtain the quantized model through PaddleSlim tool
Train the low-precision models please refer to [PaddleSlim](https://paddleslim.readthedocs.io/zh_CN/latest/tutorials/quant/overview.html).

## Deploy the quantized model from PaddleSlim using Paddle Serving with Nvidia TensorRT int8 mode

Firstly, download the [Resnet50 int8 model](https://paddle-inference-dist.bj.bcebos.com/inference_demo/python/resnet50/ResNet50_quant.tar.gz) and convert to Paddle Serving's saved model。
```
wget https://paddle-inference-dist.bj.bcebos.com/inference_demo/python/resnet50/ResNet50_quant.tar.gz
tar zxvf ResNet50_quant.tar.gz

python3 -m paddle_serving_client.convert --dirname ResNet50_quant
```
Start RPC service, specify the GPU id and precision mode
```
python3 -m paddle_serving_server.serve --model serving_server --port 9393 --gpu_ids 0 --use_trt --precision int8 
```
Request the serving service with Client
```
python3 resnet50_client.py
```

## Reference
* [PaddleSlim](https://github.com/PaddlePaddle/PaddleSlim)
* [Deploy the quantized model Using Paddle Inference on Intel CPU](https://paddle-inference.readthedocs.io/en/latest/optimize/paddle_x86_cpu_int8.html)
* [Deploy the quantized model Using Paddle Inference on Nvidia GPU](https://paddle-inference.readthedocs.io/en/latest/optimize/paddle_trt.html)
