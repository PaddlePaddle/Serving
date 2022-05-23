# How to write an general operator?

([简体中文](./OP_CN.md)|English)

In this document, we mainly focus on how to develop a new server side operator for PaddleServing. Before we start to write a new operator, let's look at some sample code to get the basic idea of writing a new operator for server. We assume you have known the basic computation logic on server side of PaddleServing, please reference to []() if you do not know much about it. The following code can be visited at `core/general-server/op` of Serving repo.

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

## Define an operator

The header file above declares a PaddleServing operator called `GeneralInferOp`. At runtime, the function `int inference()` will be called. Usually we define a server side operator to be a subclass of`baidu::paddle_serving::predictor::OpWithChannel`, and `GeneralBlob` data structure is used. 

## Use `GeneralBlob`  between operators

`GeneralBlob` is a data structure that can be used between server side operators. The `tensor_vector` is the most important data structure in `GeneralBlob`. An operator on server side can have multiple `paddle::PaddleTensor` as inputs, and have multiple `paddle::PaddleTensor` as outputs. In particular, `tensor_vector` can be feed into Paddle inference engine directly with zero copy.

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

### Implement `int Inference()`

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

`input_blob` and `output_blob` both have multiple `paddle::PaddleTensor`, and the Paddle Inference library can be called through `InferManager::instance().infer(engine_name().c_str(), in, out, batch_size)`. Most of the other code in this function is about profiling, we may remove redudant code in the future as well.

Basically, the above code can implement a new operator. If you want to visit dictionary resource, you can reference `core/predictor/framework/resource.cpp` to add global visible resources. The initialization of resources is executed at the runtime of starting server.

## Define Python API

After you have defined a C++ operator on server side for Paddle Serving, the last step is to add a registration in Python API for PaddleServing server API, `python/paddle_serving_server/dag.py` in the repo has the code piece.


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

In `python/paddle_serving_server/server.py` file, only the class name of the C++ OP class that needs to load the model and execute prediction is added. 

For example, `GeneralReaderOp`, need to be added in the 👆 code, but not in the 👇 code. Because it only does some simple data processing without loading the model and call prediction. 
``` python
default_engine_types = [
                'GeneralInferOp',
                'GeneralDistKVInferOp',
                'GeneralDistKVQuantInferOp',
                'GeneralDetectionOp',
            ]
```
