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
#ifdef BCLOUD
#ifdef WITH_GPU
#include "paddle/paddle_inference_api.h"
#else
#include "paddle/fluid/inference/api/paddle_inference_api.h"
#endif
#else
#include "paddle_inference_api.h"  // NOLINT
#endif
#include "examples/demo-serving/general_model_service.pb.h"

namespace baidu {
namespace paddle_serving {
namespace serving {

static const char* GENERAL_MODEL_NAME = "general_model";

class GeneralModelOp
    : public baidu::paddle_serving::predictor::OpWithChannel<
          baidu::paddle_serving::predictor::general_model::Response> {
 public:
  typedef std::vector<paddle::PaddleTensor> TensorVector;

  DECLARE_OP(GeneralModelOp);

  int inference();
};

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
