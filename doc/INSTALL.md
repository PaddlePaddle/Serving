# Install

## 系统需求

OS: Linux

CMake: 3.2

python

## 编译
```shell
$ git clone https://github.com/PaddlePaddle/serving.git
$ cd serving
$ mkdir build
$ cd build
$ cmake ..
$ make -j4
$ make install
```

`make install`将把目标产出放在/path/to/paddle-serving/build/output/目录下，目录结构：

```
.
|-- bin                             # Paddle Serving protobuf编译插件pdcodegen所在目录
|-- demo                            # demo总目录
|   |-- client
|   |   |-- dense_format            # dense_format客户端
|   |   |   |-- bin                 # bin/dense_format是dense_format客户端bin
|   |   |   `-- conf
|   |   |-- echo                    # echo服务客户端
|   |   |   |-- bin                 # bin/echo是echo客户端bin
|   |   |   \-- conf
|   |   |-- image_classification    # image_classification服务客户端
|   |   |   |-- bin                 # bin/ximage是image_classification客户端bin
|   |   |   |-- conf
|   |   |   |-- data
|   |   |      `-- images
|   |   |-- int64tensor_format      # int64tensor_format服务客户端
|   |   |   |-- bin                 # bin/int64tensor_format是客户端bin
|   |   |   `-- conf
|   |   `-- sparse_format           # sparse_format客户端
|   |       |-- bin                 # bin/sparse_format是客户端bin
|   |       `-- conf
|   `-- serving                     # serving端，同时提供echo/dense_format/sparse_format/int64tensor_format/image_class等5种服务
|       |-- bin                     # bin/serving是serving端可执行bin
|       |-- conf                    # 配置文件目录
|       |-- data
|       |   `-- model
|       |       `-- paddle
|       |           `-- fluid
|       |               `-- SE_ResNeXt50_32x4d  # image_classification模型
`-- lib                             # Paddle Serving产出的静态库文件: libpdseving.a, libsdk-cpp.a, libconfigure.a, libfluid_cpu_engine.a
```

如要编写新的预测服务，请参考[从零开始写一个预测服务](CREATING.md)

# CMake编译选项说明

因Paddle Serving依托于PaddlePaddle项目进行构建，以下编译选项其实是传递给PaddlePaddle的编译选项：

| 编译选项 | 说明|
|----------|-----|
| WITH_AVX | Compile PaddlePaddle with AVX intrinsics |
| WITH_MKL | Compile PaddlePaddle with MKLML library |
