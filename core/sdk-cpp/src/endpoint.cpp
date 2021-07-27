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

#include "core/sdk-cpp/include/endpoint.h"
#include "core/sdk-cpp/include/factory.h"

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

int Endpoint::initialize(const EndpointInfo& ep_info) {
  _variant_list.clear();
  _endpoint_name = ep_info.endpoint_name;
  _abtest_router = static_cast<EndpointRouterBase*>(ep_info.ab_test);
  uint32_t var_size = ep_info.vars.size();
  for (uint32_t vi = 0; vi < var_size; ++vi) {
    const VariantInfo& var_info = ep_info.vars[vi];
    Variant* var = new (std::nothrow) Variant;
    if (!var || var->initialize(ep_info, var_info) != 0) {
      LOG(ERROR) << "Failed initialize variant, tag:"
                 << var_info.parameters.route_tag.value
                 << ", endpoint: " << ep_info.endpoint_name
                 << ", var index: " << vi;
      return -1;
    }
    _variant_list.push_back(var);
    VLOG(2) << "Succ create variant: " << vi << ", endpoint:" << _endpoint_name;
  }

  return 0;
}

int Endpoint::thrd_initialize() {
  uint32_t var_size = _variant_list.size();
  for (uint32_t vi = 0; vi < var_size; ++vi) {
    Variant* var = _variant_list[vi];
    if (!var || var->thrd_initialize()) {
      LOG(ERROR) << "Failed thrd initialize var: " << vi;
      return -1;
    }
  }
  VLOG(2) << "Succ thrd initialize all vars: " << var_size;
  return 0;
}

int Endpoint::thrd_clear() {
  uint32_t var_size = _variant_list.size();
  for (uint32_t vi = 0; vi < var_size; ++vi) {
    Variant* var = _variant_list[vi];
    if (!var || var->thrd_clear()) {
      LOG(ERROR) << "Failed thrd clear var: " << vi;
      return -1;
    }
  }
  return 0;
}

int Endpoint::thrd_finalize() {
  uint32_t var_size = _variant_list.size();
  for (uint32_t vi = 0; vi < var_size; ++vi) {
    Variant* var = _variant_list[vi];
    if (!var || var->thrd_finalize()) {
      LOG(ERROR) << "Failed thrd finalize var: " << vi;
      return -1;
    }
  }
  VLOG(2) << "Succ thrd finalize all vars: " << var_size;
  return 0;
}

Predictor* Endpoint::get_predictor(std::string* variant_tag) {
  if (_variant_list.size() == 1) {
    if (_variant_list[0] == NULL) {
      LOG(ERROR) << "Not valid variant info";
      return NULL;
    }
    Variant* var = _variant_list[0];
    *variant_tag = var->variant_tag();
    return var->get_predictor();
  }

  if (_abtest_router == NULL) {
    LOG(FATAL) << "Not valid abtest_router!";
    return NULL;
  }

  Variant* var = _abtest_router->route(_variant_list);
  if (!var) {
    LOG(FATAL) << "get null var from endpoint";
    return NULL;
  }

  *variant_tag = var->variant_tag();
  return var->get_predictor();
}

int Endpoint::ret_predictor(Predictor* predictor) {
  const Stub* stub = predictor->stub();
  if (!stub || stub->return_predictor(predictor) != 0) {
    LOG(ERROR) << "Failed return predictor to pool";
    return -1;
  }

  return 0;
}

}  // namespace sdk_cpp
}  // namespace paddle_serving
}  // namespace baidu
