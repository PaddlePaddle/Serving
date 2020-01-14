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
#include "core/predictor/common/inner_common.h"
namespace baidu {
namespace paddle_serving {
namespace predictor {

using configure::ModelToolkitConf;

struct KVInfo {
  std::string model_name;
  uint32_t sparse_param_service_type;
  std::string sparse_param_service_table_name;
};

class KVManager {
 public:
  static KVManager &instance() {
    static KVManager ins;
    return ins;
  }

  int proc_initialize(const char *path, const char *file) {
    ModelToolkitConf model_toolkit_conf;
    if (configure::read_proto_conf(path, file, &model_toolkit_conf) != 0) {
      LOG(ERROR) << "failed load infer config, path: " << path << "/" << file;
      return -1;
    }

    size_t engine_num = model_toolkit_conf.engines_size();
    for (size_t ei = 0; ei < engine_num; ++ei) {
      const configure::EngineDesc &conf = model_toolkit_conf.engines(ei);
      std::string engine_name = conf.name();
      KVInfo *kvinfo = new (std::nothrow) KVInfo();
      kvinfo->model_name = engine_name;
      if (conf.has_sparse_param_service_type()) {
        kvinfo->sparse_param_service_type = conf.sparse_param_service_type();
      } else {
        kvinfo->sparse_param_service_type = configure::EngineDesc::NONE;
      }

      if (conf.has_sparse_param_service_table_name()) {
        kvinfo->sparse_param_service_table_name =
            conf.sparse_param_service_table_name();
      } else {
        kvinfo->sparse_param_service_table_name = "";
      }

      auto r = _map.insert(std::make_pair(engine_name, kvinfo));
      if (!r.second) {
        LOG(ERROR) << "Failed insert item: " << engine_name;
        return -1;
      }

      LOG(WARNING) << engine_name << ": "
                   << kvinfo->sparse_param_service_table_name;
      LOG(WARNING) << "Succ proc initialize kvmanager for engine: "
                   << engine_name;
    }

    return 0;
  }

  const KVInfo *get_kv_info(std::string model_name) {
    auto it = _map.find(model_name);
    if (it == _map.end()) {
      LOG(WARNING) << "Cannot find kvinfo for model " << model_name;
      return NULL;
    }

    return it->second;
  }

 private:
  std::map<std::string, KVInfo *> _map;
};
}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
