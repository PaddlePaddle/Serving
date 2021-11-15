# HTTP方式访问Server

Paddle Serving服务端目前提供了支持Http直接访问的功能，本文档显示了详细信息。

## 基本原理

BRPC-Server端支持通过Http的方式被访问，各种语言都有实现Http请求的一些库，所以Java/Python/Go等BRPC支持不太完善的语言，可以通过Http的方式直接访问服务端进行预测。

### Http方式
基本流程和原理：客户端需要将数据按照Proto约定的格式(请参阅[`core/general-server/proto/general_model_service.proto`](../../core/general-server/proto/general_model_service.proto))封装在Http请求的请求体中。
BRPC-Server会尝试去JSON字符串中再去反序列化出Proto格式的数据，从而进行后续的处理。

### Http+protobuf方式
各种语言都提供了对ProtoBuf的支持，如果您对此比较熟悉，您也可以先将数据使用ProtoBuf序列化，再将序列化后的数据放入Http请求数据体中，然后指定Content-Type: application/proto，从而使用http/h2+protobuf二进制串访问服务。
实测随着数据量的增大，使用JSON方式的Http的数据量和反序列化的耗时会大幅度增加，推荐当您的数据量较大时，使用Http+protobuf方式，目前已经在Java和Python的Client端提供了支持。


## 示例

我们将以examples/C++/fit_a_line为例，讲解如何通过Http访问Server端。

### 获取模型

```shell
sh get_data.sh
```

## 开启服务端

```shell
python3.6 -m paddle_serving_server.serve --model uci_housing_model --thread 10 --port 9393
```
服务端无须做任何改造，即可支持BRPC和HTTP两种方式。


## 客户端访问


### HttpClient方式发送Http请求(Python/Java)

为了方便用户快速的使用Http方式请求Server端预测服务，我们已经将常用的Http请求的数据体封装、压缩、请求加密等功能封装为一个HttpClient类提供给用户，方便用户使用。

使用HttpClient最简单只需要四步，1、创建一个HttpClient对象。2、加载Client端的prototxt配置文件（本例中为examples/C++/fit_a_line目录下的uci_housing_client/serving_client_conf.prototxt)。3、调用connect函数。4、调用Predict函数，通过Http方式请求预测服务。

此外，您可以根据自己的需要配置Server端IP、Port、服务名称（此服务名称需要与[`core/general-server/proto/general_model_service.proto`](../../core/general-server/proto/general_model_service.proto)文件中的Service服务名和rpc方法名对应，即`GeneralModelService`字段和`inference`字段），设置Request数据体压缩，设置Response支持压缩传输，模型加密预测（需要配置Server端使用模型加密）、设置响应超时时间等功能。

Python的HttpClient使用示例见[`examples/C++/fit_a_line/test_httpclient.py`](../../examples/C++/fit_a_line/test_httpclient.py)，接口详见[`python/paddle_serving_client/httpclient.py`](../../python/paddle_serving_client/httpclient.py)。

Java的HttpClient使用示例见[`java/examples/src/main/java/PaddleServingClientExample.java`](../../java/examples/src/main/java/PaddleServingClientExample.java)接口详见[`java/src/main/java/io/paddle/serving/client/Client.java`](../../java/src/main/java/io/paddle/serving/client/Client.java)。

如果不能满足您的需求，您也可以在此基础上添加一些功能。

如需支持https或者自定义Response的Status Code等,则需要对C++端brpc-Server进行一定的二次开发，请参考https://github.com/apache/incubator-brpc/blob/master/docs/cn/http_service.md

后续如果需求很大，我们也会将这部分功能加入到Server中，尽情期待。


### curl方式发送Http请求(基本原理)

```shell
curl -XPOST http://0.0.0.0:9393/GeneralModelService/inference -d ' {"tensor":[{"float_data":[0.0137,-0.1136,0.2553,-0.0692,0.0582,-0.0727,-0.1583,-0.0584,0.6283,0.4919,0.1856,0.0795,-0.0332],"elem_type":1,"name":"x","alias_name":"x","shape":[1,13]}],"fetch_var_names":["price"],"log_id":0}'
```
其中`127.0.0.1:9393`为IP和Port，根据您服务端启动的IP和Port自行设定。

`GeneralModelService`字段和`inference`字段分别为Proto文件中的Service服务名和rpc方法名，详见[`core/general-server/proto/general_model_service.proto`](../../core/general-server/proto/general_model_service.proto)

-d后面的是请求的数据体，json中一定要包含上述proto中的required字段，否则转化会失败，对应请求会被拒绝。

需要注意的是，数据中的shape字段为模型实际需要的shape信息，包含batch维度在内，可能与proto文件中的shape不一致。

#### message

对应rapidjson Object, 以花括号包围，其中的元素会被递归地解析。

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

#### repeated field

对应rapidjson Array, 以方括号包围，其中的元素会被递归地解析，和message不同，每个元素的类型相同。

```protobuf
// protobuf
repeated int32 numbers = 1;

// rapidjson
{"numbers" : [12, 17, 1, 24] }
```
#### elem_type

表示数据类型，0 means int64, 1 means float32, 2 means int32, 20 means bytes(string)

#### fetch_var_names

表示返回结果中需要的数据名称，请参考模型文件serving_client_conf.prototxt中的`fetch_var`字段下的`alias_name`。

### Http压缩

支持gzip压缩，但gzip并不是一个压缩解压速度非常快的方法，当数据量较小时候，使用gzip压缩反而会得不偿失，推荐至少数据大于512字节时才考虑使用gzip压缩,实测结果是当数据量小于50K时，压缩的收益都不大。

#### Client请求的数据体压缩

以上面的fit_a_line为例，仍使用上文的请求数据体，但只作为示例演示用法，实际此时使用压缩得不偿失。

```shell
echo ' {"tensor":[{"float_data":[0.0137,-0.1136,0.2553,-0.0692,0.0582,-0.0727,-0.1583,-0.0584,0.6283,0.4919,0.1856,0.0795,-0.0332],"elem_type":1,"shape":[1,13]}],"fetch_var_names":["price"],"log_id":0}' | gzip -c > data.txt.gz
```

```shell
curl --data-binary @data.txt.gz -H'Content-Encoding: gzip' -XPOST http://127.0.0.1:9393/GeneralModelService/inference
```

**注意：当请求数据体压缩时，需要指定请求头中Content-Encoding: gzip**

#### Server端Response压缩

当Http请求头中设置了Accept-encoding: gzip时，Server端会尝试用gzip压缩Response的数据，“尝试“指的是压缩有可能不发生，条件有：

- 请求中没有设置Accept-encoding: gzip。

- body尺寸小于-http_body_compress_threshold指定的字节数，默认是512。gzip并不是一个很快的压缩算法，当body较小时，压缩增加的延时可能比网络传输省下的还多。当包较小时不做压缩可能是个更好的选项。

这时server总是会返回不压缩的结果。

如果使用curl，通常推荐使用--compressed参数来设置Response压缩，--compressed参数会自动地在http请求中设置Accept-encoding: gzip，并在收到压缩后的Response后自动解压，对于用户而言，整个压缩/解压过程就像透明的一样。
```shell
curl --data-binary @data.txt.gz -H'Content-Encoding: gzip' --compressed -XPOST http://127.0.0.1:9393/GeneralModelService/inference
```

若您只是在Http请求头中通过-H'Accept-encoding: gzip'设置了接收压缩的信息，收到的将是压缩后的Response，此时，您需要手动解压。

也就是说，--compressed = -H'Content-Encoding: gzip' + 自动解压，所以推荐您使用--compressed，以下仅作为单独设置请求头+手动解压的原理性示例。

当您想要验证返回值是否真的压缩时，您可以只添加请求头-H'Content-Encoding: gzip'，而不解压，可以看到返回信息是压缩后的数据（一般而言是看不懂的压缩码）。
```shell
curl --data-binary @data.txt.gz -H'Content-Encoding: gzip' -H'Accept-encoding: gzip' -XPOST http://127.0.0.1:9393/GeneralModelService/inference | gunzip
```
