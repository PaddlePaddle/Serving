# Cube 编译

## 编译依赖

**以下是主要组件及其编译依赖**

|             组件             |             说明              |
| :--------------------------: | :-------------------------------: |
|             Cube-Server              |     C++程序，提供高效快速的 RPC 协议      |
|             Cube-Agent              |          Go 程序，需要 Go 环境支持         |
|           Cube-Transfer            |          Go 程序，需要 Go 环境支持         |
|            Cube-Builder             |           C++程序          |
|            Cube-Cli            |          C++组件，已集成进 C++ server 中，不需单独编译          |

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
make -j
cd ..
```

最终我们会在`build_cube/core/cube`目录下看到 Cube 组件已经编译完成，其中:
- Cube-Server：build_cube/core/cube/cube-server/cube
- Cube-Agent：build_cube/core/cube/cube-agent/src/cube-agent
- Cube-Transfer：build_cube/core/cube/cube-transfer/src/cube-transfer
- Cube-Builder：build_cube/core/cube/cube-builder/cube-builder