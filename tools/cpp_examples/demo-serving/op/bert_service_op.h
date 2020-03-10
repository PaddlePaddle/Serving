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
#include <vector>
#include "examples/demo-serving/bert_service.pb.h"
#include "paddle_inference_api.h"  // NOLINT

#include <sys/time.h>  // NOLINT

namespace baidu {
namespace paddle_serving {
namespace serving {

static const char* BERT_MODEL_NAME = "bert";

class BertServiceOp
    : public baidu::paddle_serving::predictor::OpWithChannel<
          baidu::paddle_serving::predictor::bert_service::Response> {
 public:
  typedef std::vector<paddle::PaddleTensor> TensorVector;

  DECLARE_OP(BertServiceOp);
  int inference();
};
}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
