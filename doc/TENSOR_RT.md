## Paddle Serving uses TensorRT

(English|[简体中文]((./TENSOR_RT_CN.md)))

### Background

Deploying models trained on mainstream frameworks through the tensorRT tool launched by Nvidia can greatly increase the speed of model inference, which is often at least 1 times faster than the original framework, and it also takes up more device memory. less. Therefore, it is very useful for all users who need to deploy models to master the method of deploying deep learning models with tensorRT. Paddle Serving provides comprehensive TensorRT ecological support.

### surroundings

Serving Cuda10.1 Cuda10.2 and Cuda11 versions support TensorRT.

#### Install Paddle

In [Development using Docker environment](./RUN_IN_DOCKER.md) and [Docker image list](./DOCKER_IMAGES.md), we give the development image of TensorRT. After using the mirror to start, you need to install the Paddle whl package that supports TensorRT, refer to the documentation on the home page

```
# GPU Cuda10.2 environment please execute
pip install paddlepaddle-gpu==2.0.0
```

**Note**: If your Cuda version is not 10.2, please do not execute the above commands directly, you need to refer to [Paddle official documentation-multi-version whl package list
](https://www.paddlepaddle.org.cn/documentation/docs/en/install/Tables_en.html#multi-version-whl-package-list-release)

Select the URL link of the corresponding GPU environment and install it. For example, for Python2.7 users of Cuda 10.1, please select `cp27-cp27mu` and
`cuda10.1-cudnn7.6-trt6.0.1.5` corresponding url, copy it and execute
```
pip install https://paddle-wheel.bj.bcebos.com/with-trt/2.0.0-gpu-cuda10.1-cudnn7-mkl/paddlepaddle_gpu-2.0.0.post101-cp27-cp27mu-linux_x86_64.whl
```
Since the default `paddlepaddle-gpu==2.0.0` is Cuda 10.2 and TensorRT is not built, if you need to use TensorRT on `paddlepaddle-gpu`, you need to find `cuda10 in the above multi-version whl package list .2-cudnn8.0-trt7.1.3`, download the corresponding Python version.


#### Install Paddle Serving
```
# Cuda10.2
pip install paddle-server-server==${VERSION}.post102
# Cuda 10.1
pip install paddle-server-server==${VERSION}.post101
# Cuda 11
pip install paddle-server-server==${VERSION}.post11
```

### Use TensorRT

#### RPC mode

In [Serving model example](../python/examples), we have given models that can be accelerated using TensorRT, such as [Faster_RCNN model](../python/examples/detection/faster_rcnn_r50_fpn_1x_coco) under detection

We just need
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/faster_rcnn_r50_fpn_1x_coco.tar
tar xf faster_rcnn_r50_fpn_1x_coco.tar
python -m paddle_serving_server_gpu.serve --model serving_server --port 9494 --gpu_ids 0 --use_trt
```
The TensorRT version of the faster_rcnn model server is started


#### Local Predictor mode

In [local_predictor](../python/paddle_serving_app/local_predict.py#L52), users can explicitly specify `use_trt=True` and pass it to `load_model_config`.
Other methods are no different from other Local Predictor methods, and you need to pay attention to the compatibility of the model with TensorRT.

#### Pipeline Mode

In [Pipeline mode](./PIPELINE_SERVING.md), our [imagenet example](../python/examples/pipeline/imagenet/config.yml#L23) gives the way to set TensorRT.
