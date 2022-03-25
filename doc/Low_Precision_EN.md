## Low-Precision Deployment for Paddle Serving

(English|[简体中文](./Low_Precision_CN.md))

Intel CPU supports int8 and bfloat16 models, NVIDIA TensorRT supports int8 and float16 models.

### Obtain the quantized model through PaddleSlim tool
Train the low-precision models please refer to [PaddleSlim](https://paddleslim.readthedocs.io/zh_CN/latest/tutorials/quant/overview.html).

### Deploy the quantized model from PaddleSlim using Paddle Serving with Nvidia TensorRT int8 mode

Firstly, download the [Resnet50 int8 model](https://paddle-inference-dist.bj.bcebos.com/inference_demo/python/resnet50/ResNet50_quant.tar.gz) and convert to Paddle Serving's saved model。
```
wget https://paddle-inference-dist.bj.bcebos.com/inference_demo/python/resnet50/ResNet50_quant.tar.gz
tar zxvf ResNet50_quant.tar.gz

python -m paddle_serving_client.convert --dirname ResNet50_quant
```
Start RPC service, specify the GPU id and precision mode
```
python -m paddle_serving_server.serve --model serving_server --port 9393 --gpu_ids 0 --use_trt --precision int8 
```
Request the serving service with Client
```
from paddle_serving_client import Client
from paddle_serving_app.reader import Sequential, File2Image, Resize, CenterCrop
from paddle_serving_app.reader import RGB2BGR, Transpose, Div, Normalize

client = Client()
client.load_client_config(
    "serving_client/serving_client_conf.prototxt")
client.connect(["127.0.0.1:9393"])

seq = Sequential([
    File2Image(), Resize(256), CenterCrop(224), RGB2BGR(), Transpose((2, 0, 1)),
    Div(255), Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225], True)
])

image_file = "daisy.jpg"
img = seq(image_file)
fetch_map = client.predict(feed={"image": img}, fetch=["save_infer_model/scale_0.tmp_0"])
print(fetch_map["save_infer_model/scale_0.tmp_0"].reshape(-1))
```

### Reference
* [PaddleSlim](https://github.com/PaddlePaddle/PaddleSlim)
* [Deploy the quantized model Using Paddle Inference on Intel CPU](https://paddle-inference.readthedocs.io/en/latest/optimize/paddle_x86_cpu_int8.html)
* [Deploy the quantized model Using Paddle Inference on Nvidia GPU](https://paddle-inference.readthedocs.io/en/latest/optimize/paddle_trt.html)
