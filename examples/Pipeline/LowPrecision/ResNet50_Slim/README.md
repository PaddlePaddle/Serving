# Low precsion examples of python pipeline 

Here we take the ResNet50 quantization model as an example to introduce the low-precision deployment case of Python Pipline.

## 1.Get model
```
wget https://paddle-inference-dist.bj.bcebos.com/inference_demo/python/resnet50/ResNet50_quant.tar.gz
tar zxvf ResNet50_quant.tar.gz
```

## 2.Save model var for serving
```
python3 -m paddle_serving_client.convert --dirname ResNet50_quant --serving_server serving_server --serving_client serving_client
```

## 3.Start server
```
python3 resnet50_web_service.py &>log.txt &
```

## 4.Test
```
python3 pipeline_rpc_client.py
python3 pipeline_http_client.py
```
