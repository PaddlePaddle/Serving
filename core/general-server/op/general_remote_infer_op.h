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
#include <memory>
#include <string>
#include <vector>
#include "core/general-server/general_model_service.pb.h"
#include "core/sdk-cpp/builtin_format.pb.h"
#include "core/sdk-cpp/general_model_service.pb.h"
#include "core/sdk-cpp/include/common.h"
#include "core/sdk-cpp/include/predictor_sdk.h"

namespace baidu {
namespace paddle_serving {
namespace serving {

using baidu::paddle_serving::sdk_cpp::PredictorApi;
using configure::SDKConf;
using configure::VariantConf;
using configure::Predictor;
using configure::VariantConf;

class GeneralRemoteInferOp
    : public baidu::paddle_serving::predictor::OpWithChannel<
          baidu::paddle_serving::predictor::general_model::Response> {
 public:
  DECLARE_OP(GeneralRemoteInferOp);

  int inference();

  std::shared_ptr<SDKConf> gen_desc(const std::string server_port);

  int connect(const std::string server_port);

  virtual ~GeneralRemoteInferOp();

 private:
  PredictorApi _api;
  baidu::paddle_serving::sdk_cpp::Predictor* _predictor;
  bool inited = false;
};

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
