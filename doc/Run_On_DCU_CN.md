## Paddle Serving使用海光芯片部署

Paddle Serving支持使用海光DCU进行预测部署。目前支持的ROCm版本为4.0.1。

## 安装Docker镜像
我们推荐使用docker部署Serving服务，可以直接从Paddle的官方镜像库拉取预先装有ROCm4.0.1的docker镜像。
```
# 拉取镜像
docker pull paddlepaddle/paddle:latest-dev-rocm4.0-miopen2.11

# 启动容器，注意这里的参数，例如shm-size, device等都需要配置
docker run -it --name paddle-rocm-dev --shm-size=128G \
     --device=/dev/kfd --device=/dev/dri --group-add video \
     --cap-add=SYS_PTRACE --security-opt seccomp=unconfined \
     paddlepaddle/paddle:latest-dev-rocm4.0-miopen2.11 /bin/bash

# 检查容器是否可以正确识别海光DCU设备
rocm-smi

# 预期得到以下结果：
======================= ROCm System Management Interface =======================
================================= Concise Info =================================
GPU  Temp   AvgPwr  SCLK     MCLK    Fan   Perf  PwrCap  VRAM%  GPU%  
0    50.0c  23.0W   1319Mhz  800Mhz  0.0%  auto  300.0W    0%   0%  
1    48.0c  25.0W   1319Mhz  800Mhz  0.0%  auto  300.0W    0%   0%  
2    48.0c  24.0W   1319Mhz  800Mhz  0.0%  auto  300.0W    0%   0%  
3    49.0c  27.0W   1319Mhz  800Mhz  0.0%  auto  300.0W    0%   0%  
================================================================================
============================= End of ROCm SMI Log ==============================
```

## 编译、安装
基本环境配置可参考[该文档](Compile_CN.md)进行配置。
### 编译
* 编译server部分
```
cd Serving
mkdir -p server-build-dcu && cd server-build-dcu

cmake -DPYTHON_INCLUDE_DIR=/opt/conda/include/python3.7m/ \
    -DPYTHON_LIBRARIES=/opt/conda/lib/libpython3.7m.so \
    -DPYTHON_EXECUTABLE=/opt/conda/bin/python \
    -DWITH_MKL=ON \
    -DWITH_ROCM=ON \
    -DSERVER=ON ..
make -j10
```

### 安装wheel包
编译步骤完成后，会在各自编译目录$build_dir/python/dist生成whl包，分别安装即可。例如server步骤，会在server-build-arm/python/dist目录下生成whl包, 使用命令```pip install -u xxx.whl```进行安装。


## 部署使用示例
以[resnet50](../examples/C++/PaddleClas/resnet_v2_50/README_CN.md)为例

### 启动rpc服务

启动rpc服务，基于1卡部署
```
python3 -m paddle_serving_server.serve --model resnet_v2_50_imagenet_model --port 9393 --gpu_ids 1
```

## 其他说明

### 模型实例及说明
支持海光芯片部署模型列表见[链接](https://www.paddlepaddle.org.cn/documentation/docs/zh/guides/09_hardware_support/rocm_docs/paddle_rocm_cn.html)。不同模型适配上存在差异，可能存在不支持的情况，部署使用存在问题时，欢迎以[Github issue](https://github.com/PaddlePaddle/Serving/issues)，我们会实时跟进。

### 昆仑芯片支持相关参考资料
* [海光芯片运行飞桨](https://www.paddlepaddle.org.cn/documentation/docs/zh/guides/09_hardware_support/rocm_docs/paddle_install_cn.html)