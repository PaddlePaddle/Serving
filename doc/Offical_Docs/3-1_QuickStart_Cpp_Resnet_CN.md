# C++ Serving 快速部署案例

- [模型介绍](#1)
- [部署步骤](#2)
    - [2.1 保存模型](#2.1)
    - [2.2 保存 Serving 部署的模型参数](#2.2)
    - [2.3 启动服务](#2.3)
    - [2.4 启动客户端](#2.4)

<a name="1"></a>

## 模型介绍
残差网络（ResNet)于2015年被提出，摘得 ImageNet 榜单5项第一，成绩大幅领先第二名，是 CNN 图像史上的一个里程碑。

从经验上看，网络结构层数越多，有利于复杂特征的提取，从理论上讲会取得更好的结果。但是，随着网络层数的增加，准确率会趋于饱和甚至会下降，称为退化问题（Degradation problem）。其根本原因是深层网络出现梯度消失或者梯度爆炸的问题。残差网络利用短路机制加入了残差单元，解决了退化问题。

ResNet 网络是参考了 VGG19 网络，加入残差单元，ResNet50 有50层网络。

<a name="2"></a>

## 部署步骤

前提条件是你已完成[环境安装](./2-0_Index_CN.md)步骤，并已验证环境安装成功，此处不在赘述。

克隆 Serving 仓库后，进入 `examples/C++/PaddleClas/resnet_50_vd` 目录下，已提供程序、配置和性能测试脚本。
```
git clone https://github.com/PaddlePaddle/Serving
```

按以下5个步骤操作即可实现模型部署。
- 一.获取模型
- 二.保存 Serving 部署的模型参数
- 三.启动服务
- 四.启动客户端

<a name="2.1"></a>

**一.获取模型**

下载 `ResNet50_vd` 的 推理模型，更多模型信息请阅读[ImageNet 预训练模型库](https://github.com/PaddlePaddle/PaddleClas/blob/8fa820f5c81edb1e7a2b222306a307bc27bff90f/docs/zh_CN/algorithm_introduction/ImageNet_models.md)
```
wget https://paddle-imagenet-models-name.bj.bcebos.com/dygraph/inference/ResNet50_vd_infer.tar && tar xf ResNet50_vd_infer.tar

```

<a name="2.2"></a>

**二.保存 Serving 部署的模型参数**

用 `paddle_serving_client` 把下载的推理模型转换成易于 Serving 部署的模型格式，完整信息请参考 [保存用于 Serving 部署的模型参数](./5-1_Save_Model_Params_CN.md)。

```
python3 -m paddle_serving_client.convert --dirname ./ResNet50_vd_infer/ \
                                         --model_filename inference.pdmodel  \
                                         --params_filename inference.pdiparams \
                                         --serving_server ./ResNet50_vd_serving/ \
                                         --serving_client ./ResNet50_vd_client/
```

保存参数后，会在当前文件夹多出 `ResNet50_vd_serving` 和 `ResNet50_vd_client` 的文件夹，分别用户服务端和客户端。
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

<a name="2.3"></a>

**三.启动服务**

C++ Serving 服务可以指定一个网络端口同时接收 HTTP、gRPC 和 bRPC 请求。命令参数 `--model` 指定模型路径，`--gpu_ids` 指定 GPU 卡，`--port` 指定端口。

```
python3 -m paddle_serving_server.serve --model ResNet50_vd_serving --gpu_ids 0 --port 9394
```

<a name="2.4"></a>

**四.启动客户端**

HTTP 客户端程序 `http_client.py` 创建请求参数，向服务端发起 HTTP 请求。

```
python3 http_client.py
```

RPC 客户端程序 `rpc_client.py` 创建请求参数，向服务端发起 gRPC 请求。

```
python3 rpc_client.py
```

成功运行后，模型预测的结果会打印如下：

```
prediction: daisy, probability: 0.9341399073600769
```
