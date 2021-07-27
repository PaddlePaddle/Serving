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
#include "core/sdk-cpp/include/config_manager.h"
#include "core/sdk-cpp/include/endpoint.h"
#include "core/sdk-cpp/include/endpoint_config.h"
#include "core/sdk-cpp/include/predictor.h"
#include "core/sdk-cpp/include/stub.h"

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

class PredictorApi {
 public:
  PredictorApi() {}

  int register_all();

  int create(const std::string& sdk_desc_str);

  int create(const char* path, const char* file);

  int thrd_initialize();

  int thrd_clear();

  int thrd_finalize();

  void destroy();

  static PredictorApi& instance() {
    static PredictorApi api;
    return api;
  }

  Predictor* fetch_predictor(std::string ep_name, std::string* variant_tag) {
    std::map<std::string, Endpoint*>::iterator it = _endpoints.find(ep_name);
    if (it == _endpoints.end() || !it->second) {
      LOG(ERROR) << "Failed fetch predictor:"
                 << ", ep_name: " << ep_name;
      return NULL;
    }
    return it->second->get_predictor(variant_tag);
  }

  Predictor* fetch_predictor(std::string ep_name,
                             const void* params,
                             std::string* variant_tag) {
    std::map<std::string, Endpoint*>::iterator it = _endpoints.find(ep_name);
    if (it == _endpoints.end() || !it->second) {
      LOG(ERROR) << "Failed fetch predictor:"
                 << ", ep_name: " << ep_name;
      return NULL;
    }
    return it->second->get_predictor(params, variant_tag);
  }

  int free_predictor(Predictor* predictor) {
    const Stub* stub = predictor->stub();
    if (!stub || stub->return_predictor(predictor) != 0) {
      LOG(ERROR) << "Failed return predictor via stub";
      return -1;
    }

    return 0;
  }

 private:
  EndpointConfigManager _config_manager;
  std::map<std::string, Endpoint*> _endpoints;
};

}  // namespace sdk_cpp
}  // namespace paddle_serving
}  // namespace baidu
