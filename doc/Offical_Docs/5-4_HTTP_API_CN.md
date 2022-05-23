# HTTP 方式访问 Server

Paddle Serving 服务端目前提供了支持 Http 直接访问的功能，本文档显示了详细信息。

- [基本原理](#1)
  - [1.1 HTTP 方式](#2.1)
  - [1.2 Http + protobuf 方式](#2.2)
- [示例](#2)
  - [2.1 获取模型](#2.1)
  - [2.2 开启服务端](#2.2)
- [客户端访问](#3)
  - [3.1 HttpClient 方式发送 Http 请求](#3.1)
  - [3.2 curl方式发送Http请求](#3.2)
  - [3.3 Http压缩](#3.3)

<a name="1"></a>

## 基本原理

Server 端支持通过 Http 的方式被访问，各种语言都有实现 Http 请求的一些库，下边介绍使用 Java/Python/Go 等语言通过 Http 的方式直接访问服务端进行预测的方法。

<a name="1.1"></a>

**一. Http 方式：**

基本流程和原理：客户端需要将数据按照 Proto 约定的格式(请参阅[`core/general-server/proto/general_model_service.proto`](../../core/general-server/proto/general_model_service.proto))封装在 Http 请求的请求体中。
Server 会尝试去 JSON 字符串中再去反序列化出 Proto 格式的数据，从而进行后续的处理。

<a name="1.2"></a>

**二. Http + protobuf 方式：**

各种语言都提供了对 ProtoBuf 的支持，如果您对此比较熟悉，您也可以先将数据使用 ProtoBuf 序列化，再将序列化后的数据放入 Http 请求数据体中，然后指定 Content-Type: application/proto，从而使用 http + protobuf 二进制串访问服务。
实测随着数据量的增大，使用 JSON 方式的 Http 的数据量和反序列化的耗时会大幅度增加，推荐当您的数据量较大时，使用 Http + protobuf 方式，目前已经在 Java 和 Python 的 Client 端提供了支持。

<a name="2"></a>

## 示例

我们将以 examples/C++/fit_a_line 为例，讲解如何通过 Http 访问 Server 端。

<a name="2.1"></a>

**一. 获取模型：**

```shell
sh get_data.sh
```

<a name="2.2"></a>

**二. 开启服务端：**

```shell
python3.6 -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9393
```
服务端无须做任何改造，即可支持 BRPC 和 HTTP 两种方式。


<a name="3"></a>

## 客户端访问

<a name="3.1"></a>

**一. HttpClient 方式发送 Http 请求(Python/Java)：**

为了方便用户快速的使用 Http 方式请求 Server 端预测服务，我们已经将常用的 Http 请求的数据体封装、压缩、请求加密等功能封装为一个 HttpClient 类提供给用户，方便用户使用。

使用 HttpClient 最简单只需要四步：
- 1、创建一个 HttpClient 对象。
- 2、加载 Client 端的 prototxt 配置文件（本例中为 examples/C++/fit_a_line 目录下的 uci_housing_client/serving_client_conf.prototxt)。
- 3、调用 connect 函数。
- 4、调用 Predict 函数，通过 Http 方式请求预测服务。

此外，您可以根据自己的需要配置:
- Server 端 IP、Port、服务名称
- 设置 Request 数据体压缩
- 设置 Response 支持压缩传输
- 模型加密预测（需要配置 Server 端使用模型加密）
- 设置响应超时时间等功能。

1. Python 的 HttpClient 使用示例如下：

```
from paddle_serving_client.httpclient import HttpClient
import sys
import numpy as np
import time

client = HttpClient()
client.load_client_config(sys.argv[1])

client.connect(["127.0.0.1:9393"])
fetch_list = client.get_fetch_names()

new_data = np.zeros((1, 13)).astype("float32")
new_data[0] = [0.0137, -0.1136, 0.2553, -0.0692, 0.0582, -0.0727, -0.1583, -0.0584, 0.6283, 0.4919, 0.1856, 0.0795, -0.0332]
fetch_map = client.predict(
    feed={"x": new_data}, fetch=fetch_list, batch=True)
print(fetch_map)
```

2. Java 的 HttpClient 使用示例如下：

```
boolean http_proto(String model_config_path) {
    float[] data = {0.0137f, -0.1136f, 0.2553f, -0.0692f,
        0.0582f, -0.0727f, -0.1583f, -0.0584f,
        0.6283f, 0.4919f, 0.1856f, 0.0795f, -0.0332f};
    INDArray npdata = Nd4j.createFromArray(data);
    long[] batch_shape = {1,13};
    INDArray batch_npdata = npdata.reshape(batch_shape);
    HashMap<String, Object> feed_data
        = new HashMap<String, Object>() {{
            put("x", batch_npdata);
        }};
    List<String> fetch = Arrays.asList("price");
    
    Client client = new Client();
    client.setIP("127.0.0.1");
    client.setPort("9393");
    client.loadClientConfig(model_config_path);
    String result = client.predict(feed_data, fetch, true, 0);
    
    System.out.println(result);
    return true;
}
```

Java 的 HttpClient 更多使用示例详见[`java/examples/src/main/java/PaddleServingClientExample.java`](../../java/examples/src/main/java/PaddleServingClientExample.java)接口详见[`java/src/main/java/io/paddle/serving/client/Client.java`](../../java/src/main/java/io/paddle/serving/client/Client.java)。

如果不能满足您的需求，您也可以在此基础上添加一些功能。

<a name="3.2"></a>

**二. curl方式发送Http请求：**

```shell
curl -XPOST http://0.0.0.0:9393/GeneralModelService/inference -d ' {"tensor":[{"float_data":[0.0137,-0.1136,0.2553,-0.0692,0.0582,-0.0727,-0.1583,-0.0584,0.6283,0.4919,0.1856,0.0795,-0.0332],"elem_type":1,"name":"x","alias_name":"x","shape":[1,13]}],"fetch_var_names":["price"],"log_id":0}'
```

其中 `127.0.0.1:9393` 为 IP 和 Port，根据您服务端启动的 IP 和 Port 自行设定。

`GeneralModelService`字段和`inference`字段分别为 Service 服务名和 rpc 方法名。

-d 后面的是请求的数据体，json 中一定要包含下述 proto 中的 required 字段，否则转化会失败，对应请求会被拒绝。

需要注意的是，数据中的 shape 字段为模型实际需要的 shape 信息，包含 batch 维度在内。

1. message

对应 rapidjson Object, 以花括号包围，其中的元素会被递归地解析。

```protobuf
// protobuf
message Foo {
    required string field1 = 1;
    required int32 field2 = 2;  
}
message Bar { 
    required Foo foo = 1; 
    optional bool flag = 2;
    required string name = 3;
}

// rapidjson
{"foo":{"field1":"hello", "field2":3},"name":"Tom" }
```

2. repeated field

对应 rapidjson Array, 以方括号包围，其中的元素会被递归地解析，和 message 不同，每个元素的类型相同。

```protobuf
// protobuf
repeated int32 numbers = 1;

// rapidjson
{"numbers" : [12, 17, 1, 24] }
```
3. elem_type

表示数据类型，0 means int64, 1 means float32, 2 means int32, 20 means bytes(string)

4. fetch_var_names

表示返回结果中需要的数据名称，请参考模型文件 serving_client_conf.prototxt 中的`fetch_var`字段下的`alias_name`。

<a name="3.2"></a>

**三. Http压缩：**

支持 gzip 压缩，但 gzip 并不是一个压缩解压速度非常快的方法，当数据量较小时候，使用 gzip 压缩反而会得不偿失，推荐至少数据大于 512 字节时才考虑使用 gzip 压缩,实测结果是当数据量小于 50K 时，压缩的收益都不大。

1. Client 请求的数据体压缩

以上面的 fit_a_line 为例，仍使用上文的请求数据体，但只作为示例演示用法，实际此时使用压缩得不偿失。

```shell
echo ' {"tensor":[{"float_data":[0.0137,-0.1136,0.2553,-0.0692,0.0582,-0.0727,-0.1583,-0.0584,0.6283,0.4919,0.1856,0.0795,-0.0332],"elem_type":1,"shape":[1,13]}],"fetch_var_names":["price"],"log_id":0}' | gzip -c > data.txt.gz
```

```shell
curl --data-binary @data.txt.gz -H'Content-Encoding: gzip' -XPOST http://127.0.0.1:9393/GeneralModelService/inference
```

**注意：当请求数据体压缩时，需要指定请求头中 Content-Encoding: gzip**

2. Server 端 Response 压缩

当 Http 请求头中设置了 Accept-encoding: gzip 时，Server 端会尝试用 gzip 压缩 Response 的数据，“尝试“指的是压缩有可能不发生，条件有：

- body 尺寸小于 -http_body_compress_threshold 指定的字节数，默认是 512。gzip 并不是一个很快的压缩算法，当 body 较小时，压缩增加的延时可能比网络传输省下的还多。当包较小时不做压缩可能是个更好的选项。

这时 server 总是会返回不压缩的结果。

如果使用 curl，通常推荐使用 --compressed 参数来设置 Response 压缩，--compressed 参数会自动地在 http 请求中设置 Accept-encoding: gzip，并在收到压缩后的 Response 后自动解压。

```shell
curl --data-binary @data.txt.gz -H'Content-Encoding: gzip' --compressed -XPOST http://127.0.0.1:9393/GeneralModelService/inference
```

若您只是在 Http 请求头中通过 -H'Accept-encoding: gzip' 设置了接收压缩的信息，收到的将是压缩后的 Response，此时，您需要手动解压。

也就是说，--compressed = -H'Content-Encoding: gzip' + 自动解压，所以推荐您使用 --compressed，以下仅作为单独设置请求头 + 手动解压的原理性示例。

当您想要验证返回值是否真的压缩时，您可以只添加请求头 -H'Content-Encoding: gzip'，而不解压，可以看到返回信息是压缩后的数据（一般而言是看不懂的压缩码）。

```shell
curl --data-binary @data.txt.gz -H'Content-Encoding: gzip' -H'Accept-encoding: gzip' -XPOST http://127.0.0.1:9393/GeneralModelService/inference | gunzip
```


