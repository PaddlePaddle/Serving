# 如何开发一个新的General Op?

(简体中文|[English](./OP_EN.md))

在本文档中，我们主要集中于如何为Paddle Serving开发新的服务器端运算符。 在开始编写新运算符之前，让我们看一些示例代码以获得为服务器编写新运算符的基本思想。 我们假设您已经知道Paddle Serving服务器端的基本计算逻辑。 下面的代码您可以在 Serving代码库下的 `core/general-server/op` 目录查阅。


``` c++
// Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include <string>
#include <vector>
#ifdef BCLOUD
#ifdef WITH_GPU
#include "paddle/paddle_inference_api.h"
#else
#include "paddle/fluid/inference/api/paddle_inference_api.h"
#endif
#else
#include "paddle_inference_api.h"  // NOLINT
#endif
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

## 定义一个Op

上面的头文件声明了一个名为`GeneralInferOp`的PaddleServing运算符。 在运行时，将调用函数 `int inference（)`。 通常，我们将服务器端运算符定义为baidu::paddle_serving::predictor::OpWithChannel的子类，并使用 `GeneralBlob` 数据结构。

## 在Op之间使用 `GeneralBlob` 

`GeneralBlob` 是一种可以在服务器端运算符之间使用的数据结构。 `tensor_vector`是`GeneralBlob`中最重要的数据结构。 服务器端的操作员可以将多个`paddle::PaddleTensor`作为输入，并可以将多个`paddle::PaddleTensor`作为输出。 特别是，`tensor_vector`可以在没有内存拷贝的操作下输入到Paddle推理引擎中。

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

### 实现 `int Inference()`

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

`input_blob` 和 `output_blob` 都有很多的 `paddle::PaddleTensor`, 且Paddle预测库会被 `InferManager::instance().infer(engine_name().c_str(), in, out, batch_size)`调用。此函数中的其他大多数代码都与性能分析有关，将来我们也可能会删除多余的代码。


基本上，以上代码可以实现一个新的运算符。如果您想访问字典资源，可以参考`core/predictor/framework/resource.cpp`来添加全局可见资源。资源的初始化在启动服务器的运行时执行。

## 定义 Python API

在服务器端为Paddle Serving定义C++运算符后，最后一步是在Python API中为Paddle Serving服务器API添加注册， `python/paddle_serving_server/dag.py`文件里有关于API注册的代码如下


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

在`python/paddle_serving_server/server.py`文件中仅添加`需要加载模型，执行推理预测的自定义的C++OP类的类名`。例如`GeneralReaderOp`由于只是做一些简单的数据处理而不加载模型调用预测，故在👆的代码中需要添加，而不添加在👇的代码中。
``` python
default_engine_types = [
                'GeneralInferOp',
                'GeneralDistKVInferOp',
                'GeneralDistKVQuantInferOp',
                'GeneralDetectionOp',
            ]
```
