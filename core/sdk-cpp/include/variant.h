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
#include <map>
#include <string>
#include "core/sdk-cpp/include/common.h"
#include "core/sdk-cpp/include/endpoint_config.h"
#include "core/sdk-cpp/include/predictor.h"
#include "core/sdk-cpp/include/stub.h"

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

class Variant {
  friend class VariantRouterBase;

 public:
  virtual ~Variant() {}

  Variant() : _default_stub(NULL) { _stub_map.clear(); }

  int initialize(const EndpointInfo& ep_info, const VariantInfo& var_info);

  int thrd_initialize();

  int thrd_clear();

  int thrd_finalize();

  Predictor* get_predictor(const void* params);

  Predictor* get_predictor();

  int ret_predictor(Predictor* predictor);

  const std::string& variant_tag() const { return _variant_tag; }

 private:
  std::string _endpoint_name;
  std::string _stub_service;

  std::string _variant_tag;
  std::map<std::string, Stub*> _stub_map;
  Stub* _default_stub;
};

}  // namespace sdk_cpp
}  // namespace paddle_serving
}  // namespace baidu
