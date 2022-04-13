# Cube 编译

## 概述

**Cube 一共有 4 个组件.**

- Cube-Server: Cube-Server 是 C++ 程序，是 Cube 套件的核心，负责稀疏参数索引服务的查询。
- Cube-Agent: Cube-Agent 是一段 Go 程序，负责稀疏参数配送。
- Cube-Transfer: Cube-Transfer 是一段 Go 程序，负责接受上游原始稀疏参数模型，做分片后分发给各个 Cube-Agent。
- Cube-Builder: Cube-Builder 是一段 C++ 程序，负责被 Cube-Transfer 调用并按配置生成相关分片。

## 编译依赖

**以下是主要组件及其编译依赖**

- Cube-Server: Cube-Server 最重要的编译依赖是 Baidu-RPC，其提供高效快速的 RPC 协议。这个依赖库可以复用 Paddle-Serving 的配置。 
- Cube-Agent: Cube-Agent 需要 Go 环境支持，需要按照[编译文档]()安装相关 Go 依赖库。 
- Cube-Transfer: Cube-Transfer 需要 Go 环境支持，需要按照[编译文档]()安装相关 Go 依赖库。
- Cube-Builder： Cube-Builder 依赖关系较为简单，不需要提前准备依赖项。

## 编译方法

推荐使用 Docker 编译，我们已经为您准备好了编译环境并配置好了上述编译依赖，详见[镜像环境]()。

**一. 设置 PYTHON 环境变量**

请按照如下，确定好需要编译的 Python 版本，设置对应的环境变量，一共需要设置三个环境变量，分别是 `PYTHON_INCLUDE_DIR`, `PYTHON_LIBRARIES`, `PYTHON_EXECUTABLE`。以下我们以 python 3.7为例，介绍如何设置这三个环境变量。

```
# 请自行修改至自身路径
export PYTHON_INCLUDE_DIR=/usr/local/include/python3.7m/
export PYTHON_LIBRARIES=/usr/local/lib/x86_64-linux-gnu/libpython3.7m.so
export PYTHON_EXECUTABLE=/usr/local/bin/python3.7

export GOPATH=$HOME/go
export PATH=$PATH:$GOPATH/bin

python3.7 -m pip install -r python/requirements.txt
 
go env -w GO111MODULE=on
go env -w GOPROXY=https://goproxy.cn,direct
go install github.com/grpc-ecosystem/grpc-gateway/protoc-gen-grpc-gateway@v1.15.2
go install github.com/grpc-ecosystem/grpc-gateway/protoc-gen-swagger@v1.15.2
go install github.com/golang/protobuf/protoc-gen-go@v1.4.3
go install google.golang.org/grpc@v1.33.0
go env -w GO111MODULE=auto
```

环境变量的含义如下表所示。

| cmake 环境变量         | 含义                                | 注意事项               | Docker 环境是否需要 |
|-----------------------|-------------------------------------|-------------------------------|--------------------|
| PYTHON_INCLUDE_DIR | Python.h 所在的目录，通常为 **/include/python3.7/Python.h | 如果没有找到。说明 1）没有安装开发版本的 Python，需重新安装 2）权限不足无法查看相关系统目录。                | 是(/usr/local/include/python3.7)                 |
| PYTHON_LIBRARIES         | libpython3.7.so 或 libpython3.7m.so 所在目录，通常为 /usr/local/lib  | 如果没有找到。说明 1）没有安装开发版本的 Python，需重新安装 2）权限不足无法查看相关系统目录。                | 是(/usr/local/lib/x86_64-linux-gnu/libpython3.7m.so)                 |
| PYTHON_EXECUTABLE   | python3.7 所在目录，通常为 /usr/local/bin |                | 是(/usr/local/bin/python3.7)                 |

**二. 编译**

```
mkdir build_cube
cd build_cube
cmake -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR \
    -DPYTHON_LIBRARIES=$PYTHON_LIBRARIES \
    -DPYTHON_EXECUTABLE=$PYTHON_EXECUTABLE \
    -DSERVER=ON \
    -DWITH_GPU=OFF ..
make -j20
cd ..
```

最终我们会在`build_cube/core/cube`目录下看到 Cube 组件已经编译完成。