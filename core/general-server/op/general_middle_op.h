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
#include "core/general-server/general_model_service.pb.h"

namespace baidu {
namespace paddle_serving {
namespace serving {

// GeneralMiddleOp是专门用来处理分布式千亿模型的特制OP
// 它必须跟在GeneralRemoteInferOp后面
// 主要作用是：1、将远程预测OP的输出Response(FetchVar) + FeedVar
// 2、将Response转换为Request
// 所以注册该OP时的模板参数是Request
class GeneralMiddleOp
    : public baidu::paddle_serving::predictor::OpWithChannel<
          baidu::paddle_serving::predictor::general_model::Request> {
 public:
  DECLARE_OP(GeneralMiddleOp);

  int inference();
  int print_count = 0;
};

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu

