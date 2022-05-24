# Download Wheel Packages

(English|[简体中文](./Latest_Packages_CN.md))

## Paddle-Serving-Server (x86 CPU/GPU)

Check the following table, and copy the address of hyperlink then run `pip3 install`. For example, if you want to install `paddle-serving-server-0.0.0-py3-non-any.whl`, right click the hyper link and copy the link address, the final command is `pip3 install https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server-0.0.0-py3-none-any.whl`.

|                           | develop whl                                                                                                                                                              | develop bin                                                                                                                             | stable whl                                                                                                                                                               | stable bin                                                                                                                              |
|---------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------|
| cpu-avx-mkl               | [paddle_serving_server-0.0.0-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server-0.0.0-py3-none-any.whl)                          | [serving-cpu-avx-mkl-0.0.0.tar.gz](https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-cpu-avx-mkl-0.0.0.tar.gz)                  | [paddle_serving_server-0.9.0-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server-0.9.0-py3-none-any.whl)                          | [serving-cpu-avx-mkl-0.9.0.tar.gz](https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-cpu-avx-mkl-0.9.0.tar.gz)                  |
| cpu-avx-openblas          | [paddle_serving_server-0.0.0-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server-0.0.0-py3-none-any.whl)                          | [serving-cpu-avx-openblas-0.0.0.tar.gz](https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-cpu-avx-openblas-0.0.0.tar.gz)        | [paddle_serving_server-0.9.0-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server-0.9.0-py3-none-any.whl)                          | [serving-cpu-avx-openblas-0.9.0.tar.gz](https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-cpu-avx-openblas-0.9.0.tar.gz)        |
| cpu-noavx-openblas        | [paddle_serving_server-0.0.0-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server-0.0.0-py3-none-any.whl)                          | [serving-cpu-noavx-openblas-0.0.0.tar.gz]( https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-cpu-noavx-openblas-0.0.0.tar.gz) | [paddle_serving_server-0.9.0-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server-0.9.0-py3-none-any.whl)                          | [serving-cpu-noavx-openblas-0.9.0.tar.gz]( https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-cpu-noavx-openblas-0.9.0.tar.gz) |
| cuda10.1-cudnn7-TensorRT6 | [paddle_serving_server_gpu-0.0.0.post101-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server_gpu-0.0.0.post101-py3-none-any.whl)  | [serving-gpu-101-0.0.0.tar.gz](https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-gpu-101-0.0.0.tar.gz)                          | [paddle_serving_server_gpu-0.9.0.post101-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server_gpu-0.9.0.post101-py3-none-any.whl)  | [serving-gpu-101-0.9.0.tar.gz](https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-gpu-101-0.9.0.tar.gz)                          |
| cuda10.2-cudnn7-TensorRT6 | [paddle_serving_server_gpu-0.0.0.post102-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server_gpu-0.0.0.post102-py3-none-any.whl)  | [serving-gpu-102-0.0.0.tar.gz](https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-gpu-102-0.0.0.tar.gz)                          | [paddle_serving_server_gpu-0.9.0.post102-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server_gpu-0.9.0.post102-py3-none-any.whl)  | [serving-gpu-102-0.9.0.tar.gz](https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-gpu-102-0.9.0.tar.gz)                          |
| cuda10.2-cudnn8-TensorRT7 | [paddle_serving_server_gpu-0.0.0.post1028-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server_gpu-0.0.0.post102-py3-none-any.whl) | [serving-gpu-1028-0.0.0.tar.gz](https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-gpu-1028-0.0.0.tar.gz)                     | [paddle_serving_server_gpu-0.9.0.post1028-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server_gpu-0.9.0.post102-py3-none-any.whl) | [serving-gpu-1028-0.9.0.tar.gz]( https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-gpu-1028-0.9.0.tar.gz )                     |
| cuda11.2-cudnn8-TensorRT8 | [paddle_serving_server_gpu-0.0.0.post112-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server_gpu-0.0.0.post112-py3-none-any.whl)  | [serving-gpu-112-0.0.0.tar.gz](https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-gpu-112-0.0.0.tar.gz)                       | [paddle_serving_server_gpu-0.9.0.post112-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server_gpu-0.9.0.post112-py3-none-any.whl)   | [serving-gpu-112-0.9.0.tar.gz]( https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-gpu-112-0.9.0.tar.gz )                       |

### Binary Package
for most users, we do not need to read this section. But if you deploy your Paddle Serving on a machine without network, you will encounter a problem that the binary executable tar file cannot be downloaded. Therefore, here we give you all the download links for various environment.

### How to setup SERVING_BIN offline?

- download the serving server whl package and bin package, and make sure they are for the same environment
- download the serving client whl and serving app whl, pay attention to the Python version.
- `pip install ` the serving and `tar xf ` the binary package, then `export SERVING_BIN=$PWD/serving-gpu-cuda11-0.0.0/serving` (take Cuda 11 as the example)

## paddle-serving-client 

|  | develop whl                                                                                                                                      | stable whl                                                                                                                                        |
|-----------------------|--------------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------|
| Python3.6             | [paddle_serving_client-0.0.0-cp36-none-any.whl](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_client-0.0.0-cp36-none-any.whl) | [paddle_serving_client-0.9.0-cp36-none-any.whl](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_client-0.9.0-cp36-none-any.whl)  |
| Python3.7             | [paddle_serving_client-0.0.0-cp37-none-any.whl](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_client-0.0.0-cp37-none-any.whl) | [paddle_serving_client-0.9.0-cp37-none-any.whl](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_client-0.9.0-cp37-none-any.whl)  |
| Python3.8             | [paddle_serving_client-0.0.0-cp38-none-any.whl](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_client-0.0.0-cp38-none-any.whl) | [paddle_serving_client-0.9.0-cp38-none-any.whl](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_client-0.9.0-cp38-none-any.whl)  |
| Python3.9             | [paddle_serving_client-0.0.0-cp39-none-any.whl](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_client-0.0.0-cp39-none-any.whl) | [paddle_serving_client-0.9.0-cp39-none-any.whl](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_client-0.9.0-cp38-none-any.whl)  |
## paddle-serving-app

|         | develop whl                                                                                                                              | stable whl                                                                                                                                  |
|---------|------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------|
| Python3 | [paddle_serving_app-0.0.0-py3-none-any.whl](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_app-0.0.0-py3-none-any.whl) | [ paddle_serving_app-0.9.0-py3-none-any.whl ]( https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_app-0.9.0-py3-none-any.whl) |


## Baidu Kunlun user
for kunlun user who uses arm-xpu or x86-xpu can download the wheel packages as follows. Users should use the xpu-beta docker [DOCKER IMAGES](./Docker_Images_CN.md) 
**We only support Python 3.6 for Kunlun Users.**

### Wheel Package Links

for arm kunlun user, 
```shell
# paddle-serving-server
wget https://paddle-serving.bj.bcebos.com/test-dev/whl/arm/paddle_serving_app-0.9.0-py3-none-any.whl
# paddle-serving-client
wget https://paddle-serving.bj.bcebos.com/test-dev/whl/arm/paddle_serving_client-0.9.0-cp36-none-any.whl
# paddle-serving-app
wget https://paddle-serving.bj.bcebos.com/test-dev/whl/arm/paddle_serving_server_xpu-0.9.0.post2-py3-none-any.whl

# SERVING BIN
wget https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-xpu-aarch64-0.9.0.tar.gz
```
 
for x86 xpu user, the wheel packages are here.
```shell
https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server_xpu-0.9.0.post2-py3-none-any.whl
```


