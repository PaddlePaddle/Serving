# 用于Paddle Serving的C++客户端

(简体中文|[English](./README.md))

## 请求BRPC-Server

### 服务端启动

以fit_a_line模型为例，服务端启动与常规BRPC-Server端启动命令一样。

```
cd ../../examples/C++/fit_a_line
sh get_data.sh
python -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9393
```

### 客户端预测

客户端目前支持BRPC
目前已经实现了BRPC的封装函数，详见[brpc_client.cpp](./src/brpc_client.cpp)

```
./simple_client --client_conf="uci_housing_client/serving_client_conf.prototxt" --server_port="127.0.0.1:9393" --test_type="brpc" --sample_type="fit_a_line"
```

更多示例详见[simple_client.cpp](./example/simple_client.cpp)

| Argument                                       | Type | Default                              | Description                                           |
| ---------------------------------------------- | ---- | ------------------------------------ | ----------------------------------------------------- |
| `client_conf`                                  | str  | `"serving_client_conf.prototxt"`     | Path of client conf                                   |
| `server_port`                                  | str  | `"127.0.0.1:9393"`                   | Exposed ip:port of server                             |
| `test_type`                                    | str  | `"brpc"`                             | Mode of request "brpc"                                |
| `sample_type`                                  | str  | `"fit_a_line"`                       | Type of sample include "fit_a_line,bert"              |
