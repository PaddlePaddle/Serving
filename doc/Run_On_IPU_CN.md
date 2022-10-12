## Paddle Serving使用Graphcore IPU芯片部署

Paddle Serving支持使用Graphcore IPU芯片进行预测部署，目前支持的SDK版本为2.6.0。

## 编译、安装

***1、paddlepaddle安装***

如果使用IPU相应代码需要源码编译安装paddlepaddle，编译安装paddle IPU以及paddle-inference库的过程请参考[文档](https://paddle-inference.readthedocs.io/en/master/guides/hardware_support/ipu_graphcore_cn.html)

编译完成后，请检查编译目录下的C++预测库是否生成，目录结构示例如下。

```
# 检查编译目录下的 C++ 预测库，目录结构如下
Paddle/build/paddle_inference_install_dir
├── CMakeCache.txt
├── paddle
│   ├── include                                    # C++ 预测库头文件目录
│   │   ├── crypto
│   │   ├── experimental
│   │   ├── internal
│   │   ├── paddle_analysis_config.h
│   │   ├── paddle_api.h
│   │   ├── paddle_infer_contrib.h
│   │   ├── paddle_infer_declare.h
│   │   ├── paddle_inference_api.h                 # C++ 预测库头文件
│   │   ├── paddle_mkldnn_quantizer_config.h
│   │   ├── paddle_pass_builder.h
│   │   └── paddle_tensor.h
│   └── lib
│       ├── libpaddle_inference.a                  # C++ 静态预测库文件
│       └── libpaddle_inference.so                 # C++ 动态态预测库文件
├── third_party
│   ├── install                                    # 第三方链接库和头文件
│   │   ├── cryptopp
│   │   ├── gflags
│   │   ├── glog
│   │   ├── mkldnn
│   │   ├── mklml
│   │   ├── protobuf
│   │   ├── utf8proc
│   │   └── xxhash
│   └── threadpool
│       └── ThreadPool.h
└── version.txt                                    # 预测库版本信息
```

设置环境变量`PADDLE_INFER_INSTALL_DIR`指向该目录，后续编译serving过程中将会使用到该环境变量。

***2、环境配置***
基本环境配置可参考[该文档](Compile_CN.md)进行配置。

***3、编译server***

```
mkdir -p server-build-ipu
cmake -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR \
    -DPYTHON_LIBRARIES=$PYTHON_LIBRARIES \
    -DPYTHON_EXECUTABLE=$PYTHON_EXECUTABLE \
    -DSERVER=ON \
    -DCMAKE_BUILD_TYPE=Release \
    -DWITH_MKL=ON \
    -DWITH_AVX=ON \
    -DPADDLE_INFER_INSTALL_DIR=$PADDLE_INFER_INSTALL_DIR \
    -DWITH_IPU=ON \
    -DWITH_GPU=OFF -B`pwd`/server-build-ipu
cd server-build-ipu
make -j10
```

***5、安装编译包***

编译步骤完成后，会在各自编译目录$build_dir/python/dist生成whl包，分别安装即可。例如server步骤，会在server-build-ipu/python/dist目录下生成whl包，使用命令`pip install -u xxx.whl`进行安装。

## 部署使用

以[resnet50](../examples/C++/ipu/resnet_v2_50_ipu/README_CN.md)为例。

### 启动rpc服务

启动rpc服务，使用IPU加速resnet50推理。
```
python3 -m paddle_serving_server.serve --model resnet_v2_50_imagenet_model --port 9393 --use_ipu
```

## 其他说明

### IPU芯片支持相关参考资料
* [Graphcore IPU芯片运行飞桨](https://www.paddlepaddle.org.cn/documentation/docs/zh/guides/hardware_support/ipu_docs/index_cn.html)
