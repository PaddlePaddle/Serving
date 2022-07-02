# C++ Serving 通讯协议

- [网络框架](#0)
- [Tensor](#1)
  - [1.1 构建 FLOAT32 Tensor](#1.1)
  - [1.2 构建 STRING Tensor](#1.2)
- [Request](#2)
  - [2.1 构建 Protobuf Request](#2.1)
  - [2.2 构建 Json Request](#2.2)
- [Response](#3)
  - [3.1 读取 Response 数据](#3.1)

<a name="0"></a>

## 网络框架

C++ Serving 基于 [bRPC](https://github.com/apache/incubator-brpc) 网络框架构建服务，支持 bRPC、gRPC 和 RESTful 协议请求。不限于开发语言和框架，甚至 `curl` 方式，只要按照上述协议封装数据并发送，Server 就能够接收、处理和返回结果。

对于支持的各种协议我们提供了部分的 Client SDK 示例供用户参考和使用，用户也可以根据自己的需求去开发新的 Client SDK，也欢迎用户添加其他语言/协议（例如 GRPC-Go、GRPC-C++ HTTP2-Go、HTTP2-Java 等）Client SDK 到我们的仓库供其他开发者借鉴和参考。

| 通信协议     | 速度 | 是否支持 | 是否提供Client SDK |
|-------------|-----|---------|-------------------|
| bRPC        | 最快 | 支持     | [C++]、[Python(Pybind方式)] |
| HTTP 2.0 + Proto | 快   | 不支持    |          |
| gRPC        | 较快   | 支持     | [Java]、[Python]  |
| HTTP 1.1 + Proto | 较快  | 支持     | [Java]、[Python]   |
| HTTP 1.1 + Json  | 慢   | 支持     | [Java]、[Python]、[Curl]  |


C++ Serving 请求和应答的数据格式为 protobuf，重要的结构有以下3个：

<a name="1"></a>

## Tensor

[Tensor](https://github.com/PaddlePaddle/Serving/blob/develop/core/general-server/proto/general_model_service.proto#L22) 可以装载多种类型的数据，是 Request 和 Response 的基础单元。Tensor 的定义如下：

```protobuf
message Tensor {
  // VarType: INT64
  repeated int64 int64_data = 1;

  // VarType: FP32
  repeated float float_data = 2;

  // VarType: INT32
  repeated int32 int_data = 3;

  // VarType: FP64
  repeated double float64_data = 4;

  // VarType: UINT32
  repeated uint32 uint32_data = 5;

  // VarType: BOOL
  repeated bool bool_data = 6;

  // (No support)VarType: COMPLEX64, 2x represents the real part, 2x+1
  // represents the imaginary part
  repeated float complex64_data = 7;

  // (No support)VarType: COMPLEX128, 2x represents the real part, 2x+1
  // represents the imaginary part
  repeated double complex128_data = 8;

  // VarType: STRING
  repeated string data = 9;

  // Element types:
  //   0 => INT64
  //   1 => FP32
  //   2 => INT32
  //   3 => FP64
  //   4 => INT16
  //   5 => FP16
  //   6 => BF16
  //   7 => UINT8
  //   8 => INT8
  //   9 => BOOL
  //  10 => COMPLEX64
  //  11 => COMPLEX128
  //  20 => STRING
  int32 elem_type = 10;

  // Shape of the tensor, including batch dimensions.
  repeated int32 shape = 11;

  // Level of data(LOD), support variable length data, only for fetch tensor
  // currently.
  repeated int32 lod = 12;

  // Correspond to the variable 'name' in the model description prototxt.
  string name = 13;

  // Correspond to the variable 'alias_name' in the model description prototxt.
  string alias_name = 14; // get from the Model prototxt

  // VarType: FP16, INT16, INT8, BF16, UINT8
  bytes tensor_content = 15;
};
```

Tensor 结构中重要成员 `elem_type`、`shape`、`lod` 和 `name/alias_name`。
- name/alias_name: 名称及别名，与模型配置对应
- elem_type：数据类型，当前支持FLOAT32, INT64, INT32, UINT8, INT8, FLOAT16
- shape：数据维度
- lod：变长结构 LoD(Level-of-Detail) Tensor 是 Paddle 的高级特性，是对 Tensor 的一种扩充，用于支持更自由的数据输入。详见[LOD](../LOD_CN.md)

|elem_type|类型|
|---------|----|
|0|INT64|
|1|FLOAT32|
|2|INT32|
|3|FP64|
|4|INT16|
|5|FP16|
|6|BF16|
|7|UINT8|
|8|INT8|
|9|BOOL|
|10|COMPLEX64|
|11|COMPLEX128
|20|STRING|

<a name="1.1"></a>

**一.构建 FLOAT32 Tensor**

创建 Tensor 对象，通过 `mutable_float_data::Resize()` 设置 FLOAT32 类型数据长度，通过 memcpy 函数拷贝数据。
```C
// 原始数据
std::vector<float> float_data;
Tensor *tensor = new Tensor;
// 设置维度，可以设置多维
for (uint32_t j = 0; j < float_shape.size(); ++j) {
  tensor->add_shape(float_shape[j]);
}
// 设置LOD信息
for (uint32_t j = 0; j < float_lod.size(); ++j) {
  tensor->add_lod(float_lod[j]);
}
// 设置类型、名称及别名
tensor->set_elem_type(1);
tensor->set_name(name);
tensor->set_alias_name(alias_name);
// 拷贝数据
int total_number = float_data.size();
tensor->mutable_float_data()->Resize(total_number, 0);
memcpy(tensor->mutable_float_data()->mutable_data(), float_data.data(), total_number * sizeof(float));
```

<a name="1.2"></a>

**二.构建 STRING Tensor**

创建 Tensor 对象，通过 `set_tensor_content` 设置 string 类型数据。
```C
// 原始数据
std::string string_data;
Tensor *tensor = new Tensor;
for (uint32_t j = 0; j < string_shape.size(); ++j) {
  tensor->add_shape(string_shape[j]);
}
for (uint32_t j = 0; j < string_lod.size(); ++j) {
  tensor->add_lod(string_lod[j]);
}
tensor->set_elem_type(8);
tensor->set_name(name);
tensor->set_alias_name(alias_name);
tensor->set_tensor_content(string_data);
```

<a name="2"></a>

## Request

Request 为客户端需要发送的请求数据，其以 Tensor 为基础数据单元，并包含了额外的请求信息。定义如下：

```protobuf
message Request {
  repeated Tensor tensor = 1;
  repeated string fetch_var_names = 2;
  bool profile_server = 3;
  uint64 log_id = 4;
};
```

- fetch_vat_names: 需要获取的输出数据名称，在 `GeneralResponseOP` 会根据该列表进行过滤.请参考模型文件 `serving_client_conf.prototxt` 中的 `fetch_var` 字段下的 `alias_name`。
- profile_server: 调试参数，打开时会输出性能信息
- log_id: 请求ID

当使用 bRPC 或 gRPC 进行请求时，使用 protobuf 或 Json 格式请求数据。

<a name="2.1"></a>

**一.构建 Protobuf Request**

创建 Request 对象，通过 `add_tensor` 接口来设置 Tensor。
```C
Request req;
req.set_log_id(log_id);
for (auto &name : fetch_name) {
  req.add_fetch_var_names(name);
}
// 添加Tensor
Tensor *tensor = req.add_tensor();
...
```
<a name="2.2"></a>

**二.构建 Json Request**

当使用 RESTful 请求时，可以使用 Json 格式数据，示例如下：

```JSON
{"tensor":[{"float_data":[0.0137,-0.1136,0.2553,-0.0692,0.0582,-0.0727,-0.1583,-0.0584,0.6283,0.4919,0.1856,0.0795,-0.0332],"elem_type":1,"name":"x","alias_name":"x","shape":[1,13]}],"fetch_var_names":["price"],"log_id":0}
```

可参考示例，不用修改整体结构，仅需修改数据类型和数据。

<a name="3"></a>

## Response

Response 为服务端返回给客户端的结果，包含了 Tensor 数据、错误码、错误信息等。定义如下：

```protobuf
message Response {
  repeated ModelOutput outputs = 1;
  repeated int64 profile_time = 2;
  // Error code
  int32 err_no = 3;

  // Error messages
  string err_msg = 4;
};

message ModelOutput {
  repeated Tensor tensor = 1;
  string engine_name = 2;
}
```

Response 结构中核心成员：
- profile_time：当设置 `request->set_profile_server(true)` 时，会返回性能信息
- err_no：错误码
- err_msg：错误信息
- engine_name：输出节点名称

|err_no|err_msg|
|---------|----|
|0|OK|
|-5000|"Paddle Serving Framework Internal Error."|
|-5001|"Paddle Serving Memory Alloc Error."|
|-5002|"Paddle Serving Array Overflow Error."|
|-5100|"Paddle Serving Op Inference Error."|

<a name="3.1"></a>

**一.读取 Response 数据**

读取 Response 对象中 Tensor 数据示例如下
```C
Response res;
uint32_t model_num = res.outputs_size();
for (uint32_t m_idx = 0; m_idx < model_num; ++m_idx) {
  std::string engine_name = output.engine_name();
  int idx = 0;
  // 读取tensor维度
  int shape_size = output.tensor(idx).shape_size();
  for (int i = 0; i < shape_size; ++i) {
    shape[i] = output.tensor(idx).shape(i);
  }
  // 读取LOD信息
  int lod_size = output.tensor(idx).lod_size();
  if (lod_size > 0) {
    lod.resize(lod_size);
    for (int i = 0; i < lod_size; ++i) {
      lod[i] = output.tensor(idx).lod(i);
    }
  }
  // 读取float数据
  int size = output.tensor(idx).float_data_size();
  float_data = std::vector<float>(
      output.tensor(idx).float_data().begin(),
      output.tensor(idx).float_data().begin() + size);
  // 读取int8数据
  string_data = output.tensor(idx).tensor_content();
}
```
