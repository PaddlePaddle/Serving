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

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

class EndpointConfigManager {
 public:
  static EndpointConfigManager& instance() {
    static EndpointConfigManager singleton;
    return singleton;
  }

  EndpointConfigManager()
      : _last_update_timestamp(0), _current_endpointmap_id(1) {}

  int create(const std::string& sdk_desc_str);

  int load(const std::string& sdk_desc_str);

  int create(const char* path, const char* file);

  int load();

  bool need_reload() { return false; }

  int reload() {
    if (!need_reload()) {
      LOG(INFO) << "Noneed reload endpoin config";
      return 0;
    }

    return load();
  }

  const std::map<std::string, EndpointInfo>& config() { return _ep_map; }

  const std::map<std::string, EndpointInfo>& config() const { return _ep_map; }

 private:
  int init_one_variant(const configure::VariantConf& conf,
                       VariantInfo& var);  // NOLINT

  int init_one_endpoint(const configure::Predictor& conf,
                        EndpointInfo& ep,  // NOLINT
                        const VariantInfo& default_var);

  int merge_variant(const VariantInfo& default_var,
                    const configure::VariantConf& conf,
                    VariantInfo& merged_var);  // NOLINT

  int parse_tag_values(SplitParameters& split);  // NOLINT

 private:
  std::map<std::string, EndpointInfo> _ep_map;
  std::string _endpoint_config_path;
  std::string _endpoint_config_file;
  uint32_t _last_update_timestamp;
  uint32_t _current_endpointmap_id;
};

}  // namespace sdk_cpp
}  // namespace paddle_serving
}  // namespace baidu
