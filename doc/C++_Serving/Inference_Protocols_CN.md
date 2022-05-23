# Inference Protocols

C++ Serving 基于 BRPC 进行服务构建，支持 BRPC、GRPC、RESTful 请求。请求数据为 protobuf 格式，详见 `core/general-server/proto/general_model_service.proto`。本文介绍构建请求以及解析结果的方法。

## Tensor

**一.Tensor 定义**

Tensor 可以装载多种类型的数据，是 Request 和 Response 的基础单元。Tensor 的具体定义如下：

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

- elem_type：数据类型，当前支持 FLOAT32, INT64, INT32, UINT8, INT8, FLOAT16

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

- shape：数据维度
- lod：lod 信息，LoD(Level-of-Detail) Tensor 是 Paddle 的高级特性，是对 Tensor 的一种扩充，用于支持更自由的数据输入。Lod 相关原理介绍，请参考[相关文档](../LOD_CN.md)
- name/alias_name: 名称及别名，与模型配置对应

**二.构建 Tensor 数据**

1. FLOAT32 类型 Tensor

```C
// 原始数据
std::vector<float> float_data;
Tensor *tensor = new Tensor;
// 设置维度，可以设置多维
for (uint32_t j = 0; j < float_shape.size(); ++j) {
  tensor->add_shape(float_shape[j]);
}
// 设置 LOD 信息
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
memcpy(tensor->mutable_float_data()->mutable_data(), float_datadata(), total_number * sizeof(float));
```

2. INT8 类型 Tensor

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

## Request

**一.Request 定义**

Request 为客户端需要发送的请求数据，其以 Tensor 为基础数据单元，并包含了额外的请求信息。定义如下：

```protobuf
message Request {
  repeated Tensor tensor = 1;
  repeated string fetch_var_names = 2;
  bool profile_server = 3;
  uint64 log_id = 4;
};
```

- fetch_vat_names: 需要获取的输出数据名称，在GeneralResponseOP会根据该列表进行过滤.请参考模型文件serving_client_conf.prototxt中的`fetch_var`字段下的`alias_name`。
- profile_server: 调试参数，打开时会输出性能信息
- log_id: 请求ID

**二.构建 Request**

1. Protobuf 形式

当使用 BRPC 或 GRPC 进行请求时，使用 protobuf 形式数据，构建方式如下：

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
2. Json 形式

当使用 RESTful 请求时，可以使用 Json 形式数据，具体格式如下：

```Json
{"tensor":[{"float_data":[0.0137,-0.1136,0.2553,-0.0692,0.0582,-0.0727,-0.1583,-0.0584,0.6283,0.4919,0.1856,0.0795,-0.0332],"elem_type":1,"name":"x","alias_name":"x","shape":[1,13]}],"fetch_var_names":["price"],"log_id":0}
```

## Response

**一.Response 定义**

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

- profile_time：当设置 request->set_profile_server(true) 时，会返回性能信息
- err_no：错误码，详见`core/predictor/common/constant.h`
- err_msg：错误信息，详见`core/predictor/common/constant.h`
- engine_name：输出节点名称

|err_no|err_msg|
|---------|----|
|0|OK|
|-5000|"Paddle Serving Framework Internal Error."|
|-5001|"Paddle Serving Memory Alloc Error."|
|-5002|"Paddle Serving Array Overflow Error."|
|-5100|"Paddle Serving Op Inference Error."|

**二.读取 Response 数据**

```C
uint32_t model_num = res.outputs_size();
for (uint32_t m_idx = 0; m_idx < model_num; ++m_idx) {
  std::string engine_name = output.engine_name();
  int idx = 0;
  // 读取 tensor 维度
  int shape_size = output.tensor(idx).shape_size();
  for (int i = 0; i < shape_size; ++i) {
    shape[i] = output.tensor(idx).shape(i);
  }
  // 读取 LOD 信息
  int lod_size = output.tensor(idx).lod_size();
  if (lod_size > 0) {
    lod.resize(lod_size);
    for (int i = 0; i < lod_size; ++i) {
      lod[i] = output.tensor(idx).lod(i);
    }
  }
  // 读取 float 数据
  int size = output.tensor(idx).float_data_size();
  float_data = std::vector<float>(
      output.tensor(idx).float_data().begin(),
      output.tensor(idx).float_data().begin() + size);
  // 读取 int8 数据
  string_data = output.tensor(idx).tensor_content();
}
```
