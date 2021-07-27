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
#include "core/predictor/common/inner_common.h"
#include "core/predictor/framework/channel.h"
#include "core/predictor/framework/op_repository.h"
#include "core/predictor/op/op.h"
#include "examples/demo-serving/image_class.pb.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

class WriteJsonOp
    : public OpWithChannel<
          baidu::paddle_serving::predictor::image_classification::Response> {
 public:
  DECLARE_OP(WriteJsonOp);

  int inference();
};

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
