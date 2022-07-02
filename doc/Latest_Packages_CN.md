# Wheel包下载

(简体中文|[English](./Latest_Packages_EN.md))

## Paddle-Serving-Server (x86 CPU/GPU)

查找下面表格，拷贝链接地址，并运行 `pip3 install`。例如要安装 `paddle-serving-server-0.0.0-py3-non-any.whl`, 请右键点击链接拷贝链接地址，最终命令是`pip3 install https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server-0.0.0-py3-none-any.whl`。

|                           | develop whl                                                                                                                                                              | develop bin                                                                                                                             | stable whl                                                                                                                                                               | stable bin                                                                                                                              |
|---------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------|
| cpu-avx-mkl               | [paddle_serving_server-0.0.0-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server-0.0.0-py3-none-any.whl)                          | [serving-cpu-avx-mkl-0.0.0.tar.gz](https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-cpu-avx-mkl-0.0.0.tar.gz)                  | [paddle_serving_server-0.9.0-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server-0.9.0-py3-none-any.whl)                          | [serving-cpu-avx-mkl-0.9.0.tar.gz](https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-cpu-avx-mkl-0.9.0.tar.gz)                  |
| cpu-avx-openblas          | [paddle_serving_server-0.0.0-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server-0.0.0-py3-none-any.whl)                          | [serving-cpu-avx-openblas-0.0.0.tar.gz](https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-cpu-avx-openblas-0.0.0.tar.gz)        | [paddle_serving_server-0.9.0-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server-0.9.0-py3-none-any.whl)                          | [serving-cpu-avx-openblas-0.9.0.tar.gz](https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-cpu-avx-openblas-0.9.0.tar.gz)        |
| cpu-noavx-openblas        | [paddle_serving_server-0.0.0-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server-0.0.0-py3-none-any.whl)                          | [ serving-cpu-noavx-openblas-0.0.0.tar.gz ]( https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-cpu-noavx-openblas-0.0.0.tar.gz) | [paddle_serving_server-0.9.0-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server-0.9.0-py3-none-any.whl)                          | [serving-cpu-noavx-openblas-0.9.0.tar.gz]( https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-cpu-noavx-openblas-0.9.0.tar.gz) |
| cuda10.1-cudnn7-TensorRT6 | [paddle_serving_server_gpu-0.0.0.post101-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server_gpu-0.0.0.post101-py3-none-any.whl)  | [serving-gpu-101-0.0.0.tar.gz](https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-gpu-101-0.0.0.tar.gz)                          | [paddle_serving_server_gpu-0.9.0.post101-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server_gpu-0.9.0.post101-py3-none-any.whl)  | [serving-gpu-101-0.9.0.tar.gz](https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-gpu-101-0.9.0.tar.gz)                          |
| cuda10.2-cudnn7-TensorRT6 | [paddle_serving_server_gpu-0.0.0.post102-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server_gpu-0.0.0.post102-py3-none-any.whl)  | [serving-gpu-102-0.0.0.tar.gz](https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-gpu-102-0.0.0.tar.gz)                          | [paddle_serving_server_gpu-0.9.0.post102-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server_gpu-0.9.0.post102-py3-none-any.whl)  | [serving-gpu-102-0.9.0.tar.gz](https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-gpu-102-0.9.0.tar.gz)                          |
| cuda10.2-cudnn8-TensorRT7 | [paddle_serving_server_gpu-0.0.0.post1028-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server_gpu-0.0.0.post102-py3-none-any.whl) | [ serving-gpu-1028-0.0.0.tar.gz]( https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-gpu-1028-0.0.0.tar.gz )                     | [paddle_serving_server_gpu-0.9.0.post1028-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server_gpu-0.9.0.post102-py3-none-any.whl) | [serving-gpu-1028-0.9.0.tar.gz]( https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-gpu-1028-0.9.0.tar.gz )                     |
| cuda11.2-cudnn8-TensorRT8 | [paddle_serving_server_gpu-0.0.0.post112-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server_gpu-0.0.0.post112-py3-none-any.whl) | [ serving-gpu-112-0.0.0.tar.gz]( https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-gpu-112-0.0.0.tar.gz )                       | [paddle_serving_server_gpu-0.9.0.post112-py3-none-any.whl ](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server_gpu-0.9.0.post112-py3-none-any.whl)   | [serving-gpu-112-0.9.0.tar.gz]( https://paddle-serving.bj.bcebos.com/test-dev/bin/serving-gpu-112-0.9.0.tar.gz )                       |

### 二进制包（Binary Package）
大多数用户不会用到此章节。但是如果你在无网络的环境下部署Paddle Serving，在首次启动Serving时，无法下载二进制tar文件。因此，提供多种环境二进制包的下载链接，下载后传到无网络环境的指定目录下，即可使用。

### 如何离线设置SERVING_BIN?

- 下载Serving Server Wheel包和二进制tar包，确保它们与环境是一致的
- 下载Serving Client Wheel包和Serving App wheel包, 同时注意Python版本要一致.
- `pip install ` 所有Wheel包 and `tar xf ` 二进制tar包, 然后`export SERVING_BIN=$PWD/serving-gpu-cuda11-0.0.0/serving` (以Cuda 11为例)

## paddle-serving-client Wheel包

|  | develop whl                                                                                                                                      | stable whl                                                                                                                                        |
|-----------------------|--------------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------|
| Python3.6             | [paddle_serving_client-0.0.0-cp36-none-any.whl](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_client-0.0.0-cp36-none-any.whl) | [paddle_serving_client-0.9.0-cp36-none-any.whl](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_client-0.9.0-cp36-none-any.whl)  |
| Python3.7             | [paddle_serving_client-0.0.0-cp37-none-any.whl](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_client-0.0.0-cp37-none-any.whl) | [paddle_serving_client-0.9.0-cp37-none-any.whl](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_client-0.9.0-cp37-none-any.whl)  |
| Python3.8             | [paddle_serving_client-0.0.0-cp38-none-any.whl](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_client-0.0.0-cp38-none-any.whl) | [paddle_serving_client-0.9.0-cp38-none-any.whl](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_client-0.9.0-cp38-none-any.whl)  |
| Python3.9             | [paddle_serving_client-0.0.0-cp39-none-any.whl](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_client-0.0.0-cp39-none-any.whl) | [paddle_serving_client-0.9.0-cp39-none-any.whl](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_client-0.9.0-cp39-none-any.whl)  |

## paddle-serving-app Wheel包

|         | develop whl                                                                                                                              | stable whl                                                                                                                                  |
|---------|------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------|
| Python3 | [paddle_serving_app-0.0.0-py3-none-any.whl](https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_app-0.0.0-py3-none-any.whl) | [ paddle_serving_app-0.9.0-py3-none-any.whl ]( https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_app-0.9.0-py3-none-any.whl) |


## 百度昆仑芯片
对于使用百度昆仑芯片的用户， 通过以下方式下载arm-xpu 或 x86-xpu Wheel包。选择 xpu-beta docker [DOCKER镜像](./Docker_Images_CN.md) 
**昆仑环境仅支持python36**

### Wheel包链接

适用ARM CPU环境的昆仑Wheel包：

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
 
适用于ARM & XPU 的 v0.9.0 版本 Wheel包：
```shell
https://paddle-serving.bj.bcebos.com/test-dev/whl/paddle_serving_server_xpu-0.9.0.post2-py3-none-any.whl
```



