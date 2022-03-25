## Paddle Serving使用百度昆仑芯片部署
(简体中文|[English](./Run_On_XPU_EN.md))

Paddle Serving支持使用百度昆仑芯片进行预测部署。目前支持在百度昆仑芯片和arm服务器（如飞腾 FT-2000+/64）, 或者百度昆仑芯片和Intel CPU服务器，上进行部署，后续完善对其他异构硬件服务器部署能力。

## 安装Docker镜像
我们推荐使用docker部署Serving服务，在xpu环境下可参考[Docker镜像](Docker_Images_CN.md)文档安装xpu镜像，并进一步完成编译、安装和部署等任务。

## 编译、安装
基本环境配置可参考[该文档](Compile_CN.md)进行配置。下面以飞腾FT-2000+/64机器为例进行介绍。
### 编译
* 编译server部分
```
cd Serving
mkdir -p server-build-arm && cd server-build-arm

cmake -DPYTHON_INCLUDE_DIR=/usr/include/python3.7m/ \
    -DPYTHON_LIBRARIES=/usr/lib64/libpython3.7m.so \
    -DPYTHON_EXECUTABLE=/usr/bin/python \
    -DWITH_PYTHON=ON \
    -DWITH_LITE=ON \
    -DWITH_XPU=ON \
    -DSERVER=ON ..
make -j10
```
可以执行`make install`把目标产出放在`./output`目录下，cmake阶段需添加`-DCMAKE_INSTALL_PREFIX=./output`选项来指定存放路径。在支持AVX2指令集的Intel CPU平台上请指定```-DWITH_MKL=ON```编译选项。
* 编译client部分
```
mkdir -p client-build-arm && cd client-build-arm

cmake -DPYTHON_INCLUDE_DIR=/usr/include/python3.7m/ \
    -DPYTHON_LIBRARIES=/usr/lib64/libpython3.7m.so \
    -DPYTHON_EXECUTABLE=/usr/bin/python \
    -DWITH_PYTHON=ON \
    -DWITH_LITE=ON \
    -DWITH_XPU=ON \
    -DCLIENT=ON ..

make -j10
```
* 编译app部分
```
cd Serving 
mkdir -p app-build-arm && cd app-build-arm

cmake -DPYTHON_INCLUDE_DIR=/usr/include/python3.7m/ \
    -DPYTHON_LIBRARIES=/usr/lib64/libpython3.7m.so \
    -DPYTHON_EXECUTABLE=/usr/bin/python \
    -DWITH_PYTHON=ON \
    -DWITH_LITE=ON \
    -DWITH_XPU=ON \
    -DAPP=ON ..

make -j10
```
### 安装wheel包
以上编译步骤完成后，会在各自编译目录$build_dir/python/dist生成whl包，分别安装即可。例如server步骤，会在server-build-arm/python/dist目录下生成whl包, 使用命令```pip install -u xxx.whl```进行安装。

## 请求参数说明
为了支持arm+xpu服务部署，使用Paddle-Lite加速能力，请求时需使用以下参数。
| 参数     | 参数说明                    | 备注                                                             |
| :------- | :-------------------------- | :--------------------------------------------------------------- |
| use_lite | 使用Paddle-Lite Engine      | 使用Paddle-Lite cpu预测能力                                      |
| use_xpu  | 使用Baidu Kunlun进行预测    | 该选项需要与use_lite配合使用                                     |
| ir_optim | 开启Paddle-Lite计算子图优化 | 详细见[Paddle-Lite](https://github.com/PaddlePaddle/Paddle-Lite) |
## 部署使用示例
### 下载模型
```
wget --no-check-certificate https://paddle-serving.bj.bcebos.com/uci_housing.tar.gz
tar -xzf uci_housing.tar.gz
```
### 启动rpc服务
主要有三种启动配置：
* 使用cpu+xpu部署，使用Paddle-Lite xpu优化加速能力；
* 单独使用cpu部署，使用Paddle-Lite优化加速能力；
* 使用cpu部署，不使用Paddle-Lite加速。
    
推荐使用前两种部署方式。

启动rpc服务，使用cpu+xpu部署，使用Paddle-Lite xpu优化加速能力
```
python3 -m paddle_serving_server.serve --model uci_housing_model --thread 6 --port 9292 --use_lite --use_xpu --ir_optim
```
启动rpc服务，使用cpu部署, 使用Paddle-Lite加速能力
```
python3 -m paddle_serving_server.serve --model uci_housing_model --thread 6 --port 9292 --use_lite --ir_optim
```
启动rpc服务，使用cpu部署, 不使用Paddle-Lite加速能力
```
python3 -m paddle_serving_server.serve --model uci_housing_model --thread 6 --port 9292
```
### client调用
```
from paddle_serving_client import Client
import numpy as np
client = Client()
client.load_client_config("uci_housing_client/serving_client_conf.prototxt")
client.connect(["127.0.0.1:9292"])
data = [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727,
        -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]
fetch_map = client.predict(feed={"x": np.array(data).reshape(1,13,1)}, fetch=["price"])
print(fetch_map)
```
## 其他说明

### 模型实例及说明
以下提供部分样例，其他模型可参照进行修改。
| 示例名称   | 示例链接                                                    |
| :--------- | :---------------------------------------------------------- |
| fit_a_line | [fit_a_line_xpu](../examples/C++/xpu/resnet_v2_50_xpu)     |
| resnet     | [resnet_v2_50_xpu](../examples/C++/xpu/resnet_v2_50_xpu) |

注：支持昆仑芯片部署模型列表见[链接](https://paddlelite.paddlepaddle.org.cn/introduction/support_model_list.html)。不同模型适配上存在差异，可能存在不支持的情况，部署使用存在问题时，欢迎以[Github issue](https://github.com/PaddlePaddle/Serving/issues)，我们会实时跟进。

### 昆仑芯片支持相关参考资料
* [昆仑XPU芯片运行飞桨](https://www.paddlepaddle.org.cn/documentation/docs/zh/develop/guides/xpu_docs/index_cn.html)
* [PaddleLite使用百度XPU预测部署](https://paddlelite.paddlepaddle.org.cn/demo_guides/baidu_xpu.html)
