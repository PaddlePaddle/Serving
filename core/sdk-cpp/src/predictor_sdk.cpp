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

#include "core/sdk-cpp/include/predictor_sdk.h"
#include "core/sdk-cpp/include/abtest.h"

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

int PredictorApi::register_all() {
  if (WeightedRandomRender::register_self() != 0) {
    LOG(ERROR) << "Failed register WeightedRandomRender";
    return -1;
  }

  VLOG(2) << "Succ register all components!";

  return 0;
}

int PredictorApi::create(const std::string& api_desc_str) {
  VLOG(2) << api_desc_str;
  if (register_all() != 0) {
    LOG(ERROR) << "Failed do register all!";
    return -1;
  }

  if (_config_manager.create(api_desc_str) != 0) {
    LOG(ERROR) << "Failed create config manager from desc string :"
               << api_desc_str;
    return -1;
  }

  const std::map<std::string, EndpointInfo>& map = _config_manager.config();
  std::map<std::string, EndpointInfo>::const_iterator it;
  for (it = map.begin(); it != map.end(); ++it) {
    const EndpointInfo& ep_info = it->second;
    Endpoint* ep = new (std::nothrow) Endpoint();
    if (ep->initialize(ep_info) != 0) {
      LOG(ERROR) << "Failed intialize endpoint:" << ep_info.endpoint_name;
      return -1;
    }

    if (_endpoints.find(ep_info.endpoint_name) != _endpoints.end()) {
      LOG(ERROR) << "Cannot insert duplicated endpoint:"
                 << ep_info.endpoint_name;
      return -1;
    }

    std::pair<std::map<std::string, Endpoint*>::iterator, bool> r =
        _endpoints.insert(std::make_pair(ep_info.endpoint_name, ep));
    if (!r.second) {
      LOG(ERROR) << "Failed insert endpoint:" << ep_info.endpoint_name;
      return -1;
    }

    VLOG(2) << "Succ create endpoint instance with name: "
            << ep_info.endpoint_name;
  }

  return 0;
}

int PredictorApi::create(const char* path, const char* file) {
  if (register_all() != 0) {
    LOG(ERROR) << "Failed do register all!";
    return -1;
  }

  if (_config_manager.create(path, file) != 0) {
    LOG(ERROR) << "Failed create config manager from conf:" << path << "/"
               << file;
    return -1;
  }

  const std::map<std::string, EndpointInfo>& map = _config_manager.config();
  std::map<std::string, EndpointInfo>::const_iterator it;
  for (it = map.begin(); it != map.end(); ++it) {
    const EndpointInfo& ep_info = it->second;
    Endpoint* ep = new (std::nothrow) Endpoint();
    if (ep->initialize(ep_info) != 0) {
      LOG(ERROR) << "Failed intialize endpoint:" << ep_info.endpoint_name;
      return -1;
    }

    if (_endpoints.find(ep_info.endpoint_name) != _endpoints.end()) {
      LOG(ERROR) << "Cannot insert duplicated endpoint:"
                 << ep_info.endpoint_name;
      return -1;
    }

    VLOG(2) << "endpoint name: " << ep_info.endpoint_name;

    std::pair<std::map<std::string, Endpoint*>::iterator, bool> r =
        _endpoints.insert(std::make_pair(ep_info.endpoint_name, ep));
    if (!r.second) {
      LOG(ERROR) << "Failed insert endpoint:" << ep_info.endpoint_name;
      return -1;
    }

    VLOG(2) << "Succ create endpoint instance with name: "
            << ep_info.endpoint_name;
  }

  return 0;
}

int PredictorApi::thrd_initialize() {
  std::map<std::string, Endpoint*>::const_iterator it;
  for (it = _endpoints.begin(); it != _endpoints.end(); ++it) {
    Endpoint* ep = it->second;
    if (ep->thrd_initialize() != 0) {
      LOG(ERROR) << "Failed thrd initialize endpoint:" << it->first;
      return -1;
    }

    VLOG(2) << "Succ thrd initialize endpoint:" << it->first;
  }
  return 0;
}

int PredictorApi::thrd_clear() {
  std::map<std::string, Endpoint*>::const_iterator it;
  for (it = _endpoints.begin(); it != _endpoints.end(); ++it) {
    Endpoint* ep = it->second;
    if (ep->thrd_clear() != 0) {
      LOG(ERROR) << "Failed thrd clear endpoint:" << it->first;
      return -1;
    }
  }
  return 0;
}

int PredictorApi::thrd_finalize() {
  std::map<std::string, Endpoint*>::const_iterator it;
  for (it = _endpoints.begin(); it != _endpoints.end(); ++it) {
    Endpoint* ep = it->second;
    if (ep->thrd_finalize() != 0) {
      LOG(ERROR) << "Failed thrd finalize endpoint:" << it->first;
      return -1;
    }

    VLOG(2) << "Succ thrd finalize endpoint:" << it->first;
  }
  return 0;
}

void PredictorApi::destroy() { return; }

}  // namespace sdk_cpp
}  // namespace paddle_serving
}  // namespace baidu
