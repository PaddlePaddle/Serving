# Latest Wheel Packages

## CPU server
### Python 3
```
# Compile by gcc8.2
https://paddle-serving.bj.bcebos.com/whl/paddle_serving_server-0.0.0-py3-none-any.whl
```

### Python 2
```
# Compile by gcc8.2
https://paddle-serving.bj.bcebos.com/whl/paddle_serving_server-0.0.0-py2-none-any.whl
```

## GPU server
### Python 3
```
#cuda 9.0, Compile by gcc4.8
https://paddle-serving.bj.bcebos.com/whl/paddle_serving_server_gpu-0.0.0.post9-py3-none-any.whl
#cuda 10.0, Compile by gcc4.8
https://paddle-serving.bj.bcebos.com/whl/paddle_serving_server_gpu-0.0.0.post10-py3-none-any.whl
#cuda10.1 with TensorRT 6, Compile by gcc8.2
https://paddle-serving.bj.bcebos.com/whl/paddle_serving_server_gpu-0.0.0.post101-py3-none-any.whl
#cuda10.2 with TensorRT 7, Compile by gcc8.2
https://paddle-serving.bj.bcebos.com/whl/paddle_serving_server_gpu-0.0.0.post102-py3-none-any.whl
#cuda11.0 with TensorRT 7 (beta), Compile by gcc8.2
https://paddle-serving.bj.bcebos.com/whl/paddle_serving_server_gpu-0.0.0.post11-py3-none-any.whl
```
### Python 2
```
#cuda 9.0, Compile by gcc4.8
https://paddle-serving.bj.bcebos.com/whl/paddle_serving_server_gpu-0.0.0.post9-py2-none-any.whl
#cuda 10.0, Compile by gcc4.8
https://paddle-serving.bj.bcebos.com/whl/paddle_serving_server_gpu-0.0.0.post10-py2-none-any.whl
#cuda10.1 with TensorRT 6, Compile by gcc8.2
https://paddle-serving.bj.bcebos.com/whl/paddle_serving_server_gpu-0.0.0.post101-py2-none-any.whl
#cuda10.2 with TensorRT 7, Compile by gcc8.2
https://paddle-serving.bj.bcebos.com/whl/paddle_serving_server_gpu-0.0.0.post102-py2-none-any.whl
#cuda11.0 with TensorRT 7 (beta), Compile by gcc8.2
https://paddle-serving.bj.bcebos.com/whl/paddle_serving_server_gpu-0.0.0.post11-py2-none-any.whl
```
**Tips:**  If you want to use CPU server and GPU server at the same time, you should check the gcc version,  only Cuda10.1/10.2/11 can run with CPU server owing to the same gcc version(8.2).

## Client

### Python 3.6
```
https://paddle-serving.bj.bcebos.com/whl/paddle_serving_client-0.0.0-cp36-none-any.whl
```
### Python 3.8
```
https://paddle-serving.bj.bcebos.com/whl/paddle_serving_client-0.0.0-cp38-none-any.whl
```
### Python 3.7
```
https://paddle-serving.bj.bcebos.com/whl/paddle_serving_client-0.0.0-cp37-none-any.whl
```
### Python 3.5
```
https://paddle-serving.bj.bcebos.com/whl/paddle_serving_client-0.0.0-cp35-none-any.whl
```
### Python 2.7
```
https://paddle-serving.bj.bcebos.com/whl/paddle_serving_client-0.0.0-cp27-none-any.whl
```


## App
### Python 3
```
https://paddle-serving.bj.bcebos.com/whl/paddle_serving_app-0.0.0-py3-none-any.whl
```

### Python 2
```
https://paddle-serving.bj.bcebos.com/whl/paddle_serving_app-0.0.0-py2-none-any.whl
```

## ARM user
for ARM user who uses [PaddleLite](https://github.com/PaddlePaddle/PaddleLite) can download the wheel packages as follows. And ARM user should use the xpu-beta docker [DOCKER IMAGES](./DOCKER_IMAGES.md) 
**We only support Python 3.6 for Arm Users.**

### Wheel Package Links
```
# Server 
https://paddle-serving.bj.bcebos.com/whl/xpu/paddle_serving_server_gpu-0.0.0.postarm_xpu-py3-none-any.whl
# Client
https://paddle-serving.bj.bcebos.com/whl/xpu/paddle_serving_client-0.0.0-cp36-none-any.whl 
# App
https://paddle-serving.bj.bcebos.com/whl/xpu/paddle_serving_app-0.0.0-py3-none-any.whl 
```


### Binary Package
for most users, we do not need to read this section. But if you deploy your Paddle Serving on a machine without network, you will encounter a problem that the binary executable tar file cannot be downloaded. Therefore, here we give you all the download links for various environment.

#### Bin links
```
# CPU AVX MKL
https://paddle-serving.bj.bcebos.com/bin/serving-cpu-avx-mkl-0.0.0.tar.gz
# CPU AVX OPENBLAS
https://paddle-serving.bj.bcebos.com/bin/serving-cpu-avx-openblas-0.0.0.tar.gz
# CPU NOAVX OPENBLAS
https://paddle-serving.bj.bcebos.com/bin/serving-cpu-noavx-openblas-0.0.0.tar.gz
# Cuda 9
https://paddle-serving.bj.bcebos.com/bin/serving-gpu-cuda9-0.0.0.tar.gz
# Cuda 10
https://paddle-serving.bj.bcebos.com/bin/serving-gpu-cuda10-0.0.0.tar.gz
# Cuda 10.1
https://paddle-serving.bj.bcebos.com/bin/serving-gpu-101-0.0.0.tar.gz
# Cuda 10.2
https://paddle-serving.bj.bcebos.com/bin/serving-gpu-102-0.0.0.tar.gz
# Cuda 11
https://paddle-serving.bj.bcebos.com/bin/serving-gpu-cuda11-0.0.0.tar.gz
```

#### How to setup SERVING_BIN offline?

- download the serving server whl package and bin package, and make sure they are for the same environment
- download the serving client whl and serving app whl, pay attention to the Python version.
- `pip install ` the serving and `tar xf ` the binary package, then `export SERVING_BIN=$PWD/serving-gpu-cuda10-0.0.0/serving` (take Cuda 10.0 as the example)

