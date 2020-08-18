# 通过 gRPC-gateway 开启 HTTP 服务

## 安装依赖

### 1. Protobuf 3

在我们提供的容器中需要先安装 `autoconf` ，`automake`，`libtool` ：

```shell
yum install -y autoconf automake libtool
```

编译 Protobuf 3：

```shell
mkdir tmp
cd tmp
git clone https://github.com/google/protobuf
cd protobuf && git submodule update --init --recursive
./autogen.sh
./configure
make
make check
sudo make install
```

### 2. Go packages

设置 `GOPATH`：
```shell
export GOPATH=$HOME/go
export PATH=$PATH:$GOPATH/bin
```
下载 packages：
```shell
go get -u github.com/grpc-ecosystem/grpc-gateway/protoc-gen-grpc-gateway
go get -u github.com/grpc-ecosystem/grpc-gateway/protoc-gen-swagger
go get -u github.com/golang/protobuf/protoc-gen-go
```

## 生成 gRPC stub 和 反向代理

```shell
cd gateway
sh gen_code.sh
```

## 编译 Go 程序

```
go build proxy_server.go
```

## 运行测试

这里以 imdb model ensemble 为例：
```shell
cd ../imdb_model_ensemble/
sh get_data.sh
python -m paddle_serving_server_gpu.serve --model imdb_cnn_model --port 9292 &> cnn.log &
python -m paddle_serving_server_gpu.serve --model imdb_bow_model --port 9393 &> bow.log &
python test_pipeline_server.py &> pipeline.log &
cd -
./proxy_server &> gateway.log &

curl -X POST -k http://localhost:8080/pipeline/prediction -d '{"key": ["words"], "value": ["i am very sad | 0"]}'
```
