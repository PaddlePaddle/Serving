# RPC 方式访问 Server

Paddle Serving 采用[brpc框架](https://github.com/apache/incubator-brpc)进行 Client/Server 端的通信。brpc 是百度开源的一款PRC网络框架，具有高并发、低延时等特点，已经支持了包括百度在内上百万在线预估实例、上千个在线预估服务，稳定可靠。与 gRPC 网络框架相比，具有更低的延时，更高的并发性能，且底层支持<mark>**brpc/grpc/http+json/http+proto**</mark>等多种协议。本文主要介绍如何使用 BRPC 进行通信。

- [示例](#1)
  - [1.1 获取模型](#1.1)
  - [1.2 开启服务端](#1.2)
- [客户端请求](#2)
  - [2.1 C++ 方法](#2.1)
  - [2.2 Python 方法](#2.2)

<a name="1"></a>

## 示例

我们将以 examples/C++/fit_a_line 为例，讲解如何通过 RPC 访问 Server 端。

<a name="1.1"></a>

**一. 获取模型：**

```shell
sh get_data.sh
```

<a name="1.2"></a>

**二. 开启服务端：**

```shell
python3.6 -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9393
```
服务端无须做任何改造，即可支持 RPC 方式。

<a name="2"></a>

## 客户端请求

<a name="2.1"></a>

**一. C++ 方法：**

基础使用方法主要分为四步：
- 1、创建一个 Client 对象。
- 2、加载 Client 端的 prototxt 配置文件（本例中为 examples/C++/fit_a_line 目录下的 uci_housing_client/serving_client_conf.prototxt)。
- 3、准备请求数据。
- 4、调用 predict 函数，通过 brpc 方式请求预测服务。
示例如下：

```
  std::unique_ptr<ServingClient> client;
  client.reset(new ServingBrpcClient());

  std::vector<std::string> confs;
  confs.push_back(conf);
  if (client->init(confs, url) != 0) {
    LOG(ERROR) << "Failed to init client!";
    return 0;
  }

  PredictorInputs input;
  PredictorOutputs output;
  std::vector<std::string> fetch_name;
  std::vector<float> float_feed = {0.0137f, -0.1136f, 0.2553f, -0.0692f,
            0.0582f, -0.0727f, -0.1583f, -0.0584f,
            0.6283f, 0.4919f, 0.1856f, 0.0795f, -0.0332f};
  std::vector<int> float_shape = {1, 13};
  std::string feed_name = "x";
  fetch_name = {"price"};
  std::vector<int> lod;
  input.add_float_data(float_feed, feed_name, float_shape, lod);

  if (client->predict(input, output, fetch_name, 0) != 0) {
    LOG(ERROR) << "Failed to predict!";
  }
  else {
    LOG(INFO) << output.print();
  }
```

具体使用详见[simple_client.cpp](./example/simple_client.cpp)，已提供封装好的调用方法。
```shell
./simple_client --client_conf="uci_housing_client/serving_client_conf.prototxt" --server_port="127.0.0.1:9393" --test_type="brpc" --sample_type="fit_a_line"
```

| Argument                                       | Type | Default                              | Description                                           |
| ---------------------------------------------- | ---- | ------------------------------------ | ----------------------------------------------------- |
| `client_conf`                                  | str  | `"serving_client_conf.prototxt"`     | Path of client conf                                   |
| `server_port`                                  | str  | `"127.0.0.1:9393"`                   | Exposed ip:port of server                             |
| `test_type`                                    | str  | `"brpc"`                             | Mode of request "brpc"                                |
| `sample_type`                                  | str  | `"fit_a_line"`                       | Type of sample include "fit_a_line,bert"              |

<a name="2.2"></a>

**二. Python 方法：**

为了方便用户快速的使用 RPC 方式请求 Server 端预测服务，我们已经将常用的 RPC 请求的数据体封装、压缩、请求加密等功能封装为一个 Client 类提供给用户，方便用户使用。

使用 Client 最简单只需要四步：1、创建一个 Client 对象。2、加载 Client 端的 prototxt 配置文件（本例中为 examples/C++/fit_a_line 目录下的 uci_housing_client/serving_client_conf.prototxt)。3、调用 connect 函数。4、调用 Predict 函数，通过 RPC 方式请求预测服务。

```
from paddle_serving_client import Client
import sys
import numpy as np

client = Client()
client.load_client_config(sys.argv[1])
client.connect(["127.0.0.1:9393"])
fetch_list = client.get_fetch_names()

new_data = np.zeros((1, 13)).astype("float32")
new_data[0] = [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]
fetch_map = client.predict(
    feed={"x": new_data}, fetch=fetch_list, batch=True)
print(fetch_map)
```