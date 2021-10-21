// Copyright (c) 2021 PaddlePaddle Authors. All Rights Reserved.
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

#include "core/general-client/include/client.h"
#include "core/sdk-cpp/include/predictor_sdk.h"
using baidu::paddle_serving::sdk_cpp::Predictor;
using baidu::paddle_serving::sdk_cpp::PredictorApi;

namespace baidu {
namespace paddle_serving {
namespace client {

class ServingBrpcClient : public ServingClient {
 public:

  ServingBrpcClient() {};

  ~ServingBrpcClient() {};

  virtual int connect(const std::string server_port);

  int predict(const PredictorInputs& inputs,
              PredictorOutputs& outputs,
              const std::vector<std::string>& fetch_name,
              const uint64_t log_id);

 private:
  // generate default SDKConf
  std::string gen_desc(const std::string server_port);

 private:
  PredictorApi _api;
  Predictor* _predictor;
};

}  // namespace client
}  // namespace paddle_serving
}  // namespace baidu