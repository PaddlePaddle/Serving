## Paddle Serving 使用 TensorRT

([English](./TENSOR_RT.md)|简体中文)

### 背景

通过Nvidia推出的tensorRT工具来部署主流框架上训练的模型能够极大的提高模型推断的速度，往往相比与原本的框架能够有至少1倍以上的速度提升，同时占用的设备内存也会更加的少。因此对是所有需要部署模型的用户来说，掌握用tensorRT来部署深度学习模型的方法是非常有用的。Paddle Serving提供了全面的TensorRT生态支持。

### 环境

Serving 的Cuda10.1 Cuda10.2和Cuda11版本支持TensorRT。

#### 安装Paddle

在[使用Docker环境开发](./RUN_IN_DOCKER_CN.md) 和 [Docker镜像列表](./DOCKER_IMAGES_CN.md)当中，我们给出了TensorRT的开发镜像。使用镜像启动之后，需要安装支持TensorRT的Paddle whl包，参考首页的文档

```
# GPU Cuda10.2环境请执行
pip install paddlepaddle-gpu==2.0.0
```

**注意**： 如果您的Cuda版本不是10.2，请勿直接执行上述命令，需要参考[Paddle官方文档-多版本whl包列表
](https://www.paddlepaddle.org.cn/documentation/docs/zh/install/Tables.html#whl-release)

选择相应的GPU环境的url链接并进行安装，例如Cuda 10.1的Python2.7用户，请选择表格当中的`cp27-cp27mu`和
`cuda10.1-cudnn7.6-trt6.0.1.5`对应的url，复制下来并执行
```
pip install https://paddle-wheel.bj.bcebos.com/with-trt/2.0.0-gpu-cuda10.1-cudnn7-mkl/paddlepaddle_gpu-2.0.0.post101-cp27-cp27mu-linux_x86_64.whl
```
由于默认的`paddlepaddle-gpu==2.0.0`是Cuda 10.2，并没有联编TensorRT，因此如果需要和在`paddlepaddle-gpu`上使用TensorRT，需要在上述多版本whl包列表当中，找到`cuda10.2-cudnn8.0-trt7.1.3`，下载对应的Python版本。


#### 安装Paddle Serving
```
# Cuda10.2
pip install paddle-server-server==${VERSION}.post102
# Cuda 10.1
pip install paddle-server-server==${VERSION}.post101
# Cuda 11
pip install paddle-server-server==${VERSION}.post11
```

### 使用TensorRT

#### RPC模式

在[Serving模型示例](../python/examples)当中，我们有给出可以使用TensorRT加速的模型，例如detection下的[Faster_RCNN模型](../python/examples/detection/faster_rcnn_r50_fpn_1x_coco)

我们只需
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/pddet_demo/2.0/faster_rcnn_r50_fpn_1x_coco.tar
tar xf faster_rcnn_r50_fpn_1x_coco.tar
python -m paddle_serving_server.serve --model serving_server --port 9494 --gpu_ids 0 --use_trt
```
TensorRT版本的faster_rcnn模型服务端就启动了


#### Local Predictor模式

在 [local_predictor](../python/paddle_serving_app/local_predict.py#L52)当中，用户可以显式制定`use_trt=True`传入到`load_model_config`当中。
其他方式和其他Local Predictor使用方法没有区别，需要注意模型对TensorRT的兼容性。

#### Pipeline模式

在 [Pipeline模式](./PIPELINE_SERVING_CN.md)当中，我们的[imagenet例子](../python/examples/pipeline/imagenet/config.yml#L23)给出了设置TensorRT的方式。


