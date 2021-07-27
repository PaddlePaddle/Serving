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
#include "core/sdk-cpp/include/abtest.h"
#include "core/sdk-cpp/include/common.h"
#include "core/sdk-cpp/include/endpoint_config.h"
#include "core/sdk-cpp/include/predictor.h"
#include "core/sdk-cpp/include/stub.h"
#include "core/sdk-cpp/include/variant.h"

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

class Endpoint {
  friend class EndpointRouterBase;

 public:
  virtual ~Endpoint() {}

  Endpoint() { _variant_list.clear(); }

  int initialize(const EndpointInfo& ep_info);

  int thrd_initialize();

  int thrd_clear();

  int thrd_finalize();

  Predictor* get_predictor(const void* params, std::string* variant_tag);

  Predictor* get_predictor(std::string* variant_tag);

  int ret_predictor(Predictor* predictor);

  const std::string& endpoint_name() const { return _endpoint_name; }

 private:
  int initialize_variant(const VariantInfo& var_info,
                         const std::string& service,
                         const std::string& ep_name,
                         std::vector<Stub*>& stubs);  // NOLINT

 private:
  std::string _endpoint_name;
  std::vector<Variant*> _variant_list;
  EndpointRouterBase* _abtest_router;
};

}  // namespace sdk_cpp
}  // namespace paddle_serving
}  // namespace baidu
