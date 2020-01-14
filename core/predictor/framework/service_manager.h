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
#include <utility>
#include "core/predictor/common/inner_common.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

#define REGIST_FORMAT_SERVICE(svr_name, svr)                                 \
  do {                                                                       \
    int ret =                                                                \
        ::baidu::paddle_serving::predictor::FormatServiceManager::instance() \
            .regist_service(svr_name, svr);                                  \
    if (ret != 0) {                                                          \
      RAW_LOG_ERROR("Failed regist service[%s][%s]",                         \
                    svr_name.c_str(),                                        \
                    typeid(svr).name());                                     \
    } else {                                                                 \
      RAW_LOG_INFO("Success regist service[%s][%s]",                         \
                   svr_name.c_str(),                                         \
                   typeid(svr).name());                                      \
    }                                                                        \
  } while (0)

class FormatServiceManager {
 public:
  typedef google::protobuf::Service Service;

  int regist_service(const std::string& svr_name, Service* svr) {
    if (_service_map.find(svr_name) != _service_map.end()) {
      RAW_LOG_ERROR("Service[%s][%s] already exist!",
                    svr_name.c_str(),
                    typeid(svr).name());
      return -1;
    }

    std::pair<boost::unordered_map<std::string, Service*>::iterator, bool> ret;
    ret = _service_map.insert(std::make_pair(svr_name, svr));
    if (ret.second == false) {
      RAW_LOG_ERROR("Service[%s][%s] insert failed!",
                    svr_name.c_str(),
                    typeid(svr).name());
      return -1;
    }

    RAW_LOG_INFO("Service[%s] insert successfully!", svr_name.c_str());
    return 0;
  }

  Service* get_service(const std::string& svr_name) {
    boost::unordered_map<std::string, Service*>::iterator res;
    if ((res = _service_map.find(svr_name)) == _service_map.end()) {
      RAW_LOG_WARNING("Service[%s] not found in service manager!",
                      svr_name.c_str());
      return NULL;
    }
    return (*res).second;
  }

  static FormatServiceManager& instance() {
    static FormatServiceManager service_;
    return service_;
  }

 private:
  boost::unordered_map<std::string, Service*> _service_map;
};

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
