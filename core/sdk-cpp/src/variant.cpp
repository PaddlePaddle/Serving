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

#include "core/sdk-cpp/include/variant.h"
#include "core/sdk-cpp/include/factory.h"

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

int Variant::initialize(const EndpointInfo& ep_info,
                        const VariantInfo& var_info) {
  _endpoint_name = ep_info.endpoint_name;
  _stub_service = ep_info.stub_service;

  _variant_tag = var_info.parameters.route_tag.value;
  _stub_map.clear();

  const SplitParameters& split_info = var_info.splitinfo;
  uint32_t tag_size = split_info.tag_values.size();
  for (uint32_t ti = 0; ti < tag_size; ++ti) {  // split
    Stub* stub = StubFactory::instance().generate_object(_stub_service);
    const std::string& tag_value = split_info.tag_values[ti];
    if (!stub ||
        stub->initialize(var_info,
                         ep_info.endpoint_name,
                         &split_info.split_tag.value,
                         &tag_value) != 0) {
      LOG(ERROR) << "Failed init stub from factory"
                 << ", stub name: " << ep_info.stub_service
                 << ", filter tag: " << tag_value;
      return -1;
    }

    // 判重
    std::map<std::string, Stub*>::iterator iter = _stub_map.find(tag_value);
    if (iter != _stub_map.end()) {
      LOG(ERROR) << "duplicated tag value: " << tag_value;
      return -1;
    }
    _stub_map[tag_value] = stub;
  }

  if (_stub_map.size() > 0) {
    VLOG(2) << "Initialize variants from VariantInfo"
            << ", stubs count: " << _stub_map.size();
    return 0;
  }

  Stub* stub = StubFactory::instance().generate_object(ep_info.stub_service);
  if (!stub || stub->initialize(var_info, _endpoint_name, NULL, NULL) != 0) {
    LOG(ERROR) << "Failed init stub from factory"
               << ", stub name: " << ep_info.stub_service;
    return -1;
  }

  _default_stub = stub;
  VLOG(2) << "Succ create default debug";
  return 0;
}

int Variant::thrd_initialize() {
  if (_stub_map.size() <= 0) {
    return _default_stub->thrd_initialize();
  }

  std::map<std::string, Stub*>::iterator iter;
  for (iter = _stub_map.begin(); iter != _stub_map.end(); ++iter) {
    Stub* stub = iter->second;
    if (!stub || stub->thrd_initialize() != 0) {
      LOG(ERROR) << "Failed thrd initialize stub: " << iter->first;
      return -1;
    }
    VLOG(2) << "Succ thrd initialize stub:" << iter->first;
  }

  VLOG(2) << "Succ thrd initialize all stubs";
  return 0;
}

int Variant::thrd_clear() {
  if (_stub_map.size() <= 0) {
    return _default_stub->thrd_clear();
  }

  std::map<std::string, Stub*>::iterator iter;
  for (iter = _stub_map.begin(); iter != _stub_map.end(); ++iter) {
    Stub* stub = iter->second;
    if (!stub || stub->thrd_clear() != 0) {
      LOG(ERROR) << "Failed thrd clear stub: " << iter->first;
      return -1;
    }
  }
  return 0;
}

int Variant::thrd_finalize() {
  if (_stub_map.size() <= 0) {
    return _default_stub->thrd_finalize();
  }

  std::map<std::string, Stub*>::iterator iter;
  for (iter = _stub_map.begin(); iter != _stub_map.end(); ++iter) {
    Stub* stub = iter->second;
    if (!stub || stub->thrd_finalize() != 0) {
      LOG(ERROR) << "Failed thrd finalize stub: " << iter->first;
      return -1;
    }
  }
  return 0;
}

Predictor* Variant::get_predictor() {
  if (_default_stub) {
    return _default_stub->fetch_predictor();
  }

  return NULL;
}

Predictor* Variant::get_predictor(const void* params) {
  if (_default_stub) {
    return _default_stub->fetch_predictor();
  }

  return NULL;
}

}  // namespace sdk_cpp
}  // namespace paddle_serving
}  // namespace baidu
