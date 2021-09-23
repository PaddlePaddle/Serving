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
// 原理上，GeneralSingleOutOp是通用类型的OP
// 适用于无论多少个前置依赖OP，只取一个而不合并的情况。
// 后续其实该OP可以通过OP中带更多的属性设置+动态类型推断（或直接取消GeneralBlob）
// 从而与GeneralResponse合并在一起。

// 目前它必须跟在GeneralRemoteInferOp后面
// 主要作用是：只取其中一个输出结果，而不合并。
// 所以注册该OP时的模板参数是Response
class GeneralSingleOutOp
    : public baidu::paddle_serving::predictor::OpWithChannel<
          baidu::paddle_serving::predictor::general_model::Response> {
 public:
  DECLARE_OP(GeneralSingleOutOp);

  int inference();
};

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
