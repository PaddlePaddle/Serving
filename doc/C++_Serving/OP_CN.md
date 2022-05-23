# 如何开发一个新的General Op?

- [定义一个Op](#1)
- [在Op之间使用 `GeneralBlob`](#2)
  - [2.1 实现 `int Inference()`](#2.1)
- [定义 Python API](#3)

在本文档中，我们主要集中于如何为 Paddle Serving 开发新的服务器端运算符。在开始编写新运算符之前，让我们看一些示例代码以获得为服务器编写新运算符的基本思想。我们假设您已经知道 Paddle Serving 服务器端的基本计算逻辑。 下面的代码您可以在 Serving代码库下的 `core/general-server/op` 目录查阅。


``` c++

#pragma once
#include <string>
#include <vector>
#include "paddle_inference_api.h"  // NOLINT
#include "core/general-server/general_model_service.pb.h"
#include "core/general-server/op/general_infer_helper.h"

namespace baidu {
namespace paddle_serving {
namespace serving {

class GeneralInferOp
    : public baidu::paddle_serving::predictor::OpWithChannel<GeneralBlob> {
 public:
  typedef std::vector<paddle::PaddleTensor> TensorVector;

  DECLARE_OP(GeneralInferOp);

  int inference();

};

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
```
<a name="1"></a>

## 定义一个Op

上面的头文件声明了一个名为 `GeneralInferOp` 的 Paddle Serving 运算符。 在运行时，将调用函数 `int inference（)`。 通常，我们将服务器端运算符定义为baidu::paddle_serving::predictor::OpWithChannel 的子类，并使用 `GeneralBlob` 数据结构。

<a name="2"></a>

## 在Op之间使用 `GeneralBlob` 

`GeneralBlob` 是一种可以在服务器端运算符之间使用的数据结构。 `tensor_vector` 是 `GeneralBlob` 中最重要的数据结构。 服务器端的操作员可以将多个 `paddle::PaddleTensor` 作为输入，并可以将多个 `paddle::PaddleTensor `作为输出。 特别是，`tensor_vector` 可以在没有内存拷贝的操作下输入到 Paddle 推理引擎中。

``` c++
struct GeneralBlob {
  std::vector<paddle::PaddleTensor> tensor_vector;
  int64_t time_stamp[20];
  int p_size = 0;

  int _batch_size;

  void Clear() {
    size_t tensor_count = tensor_vector.size();
    for (size_t ti = 0; ti < tensor_count; ++ti) {
      tensor_vector[ti].shape.clear();
    }
    tensor_vector.clear();
  }

  int SetBatchSize(int batch_size) { _batch_size = batch_size; }

  int GetBatchSize() const { return _batch_size; }
  std::string ShortDebugString() const { return "Not implemented!"; }
};
```

<a name="2.1"></a>

**一. 实现 `int Inference()`**

``` c++
int GeneralInferOp::inference() {
  VLOG(2) << "Going to run inference";
  const GeneralBlob *input_blob = get_depend_argument<GeneralBlob>(pre_name());
  VLOG(2) << "Get precedent op name: " << pre_name();
  GeneralBlob *output_blob = mutable_data<GeneralBlob>();

  if (!input_blob) {
    LOG(ERROR) << "Failed mutable depended argument, op:" << pre_name();
    return -1;
  }

  const TensorVector *in = &input_blob->tensor_vector;
  TensorVector *out = &output_blob->tensor_vector;
  int batch_size = input_blob->GetBatchSize();
  VLOG(2) << "input batch size: " << batch_size;

  output_blob->SetBatchSize(batch_size);

  VLOG(2) << "infer batch size: " << batch_size;

  Timer timeline;
  int64_t start = timeline.TimeStampUS();
  timeline.Start();

  if (InferManager::instance().infer(engine_name().c_str(), in, out, batch_size)) {
    LOG(ERROR) << "Failed do infer in fluid model: " << engine_name().c_str();
    return -1;
  }

  int64_t end = timeline.TimeStampUS();
  CopyBlobInfo(input_blob, output_blob);
  AddBlobInfo(output_blob, start);
  AddBlobInfo(output_blob, end);
  return 0;
}
DEFINE_OP(GeneralInferOp);
```

`input_blob` 和 `output_blob` 都有很多的 `paddle::PaddleTensor`, 且 Paddle 预测库会被 `InferManager::instance().infer(engine_name().c_str(), in, out, batch_size)` 调用。此函数中的其他大多数代码都与性能分析有关，将来我们也可能会删除多余的代码。

<a name="3"></a>

## 定义 Python API

在服务器端为 Paddle Serving 定义 C++ 运算符后，最后一步是在 Python API 中为 Paddle Serving 服务器 API 添加注册， `python/paddle_serving_server/dag.py` 文件里有关于 API 注册的代码如下


``` python
self.op_list = [
            "GeneralInferOp",
            "GeneralReaderOp",
            "GeneralResponseOp",
            "GeneralTextReaderOp",
            "GeneralTextResponseOp",
            "GeneralSingleKVOp",
            "GeneralDistKVInferOp",
            "GeneralDistKVOp",
            "GeneralCopyOp",
            "GeneralDetectionOp",
        ]
```

在 `python/paddle_serving_server/server.py` 文件中仅添加`需要加载模型，执行推理预测的自定义的 C++ OP 类的类名`。例如 `GeneralReaderOp` 由于只是做一些简单的数据处理而不加载模型调用预测，故在上述的代码中需要添加，而不添加在下方的代码中。
``` python
default_engine_types = [
                'GeneralInferOp',
                'GeneralDistKVInferOp',
                'GeneralDistKVQuantInferOp',
                'GeneralDetectionOp',
            ]
```
