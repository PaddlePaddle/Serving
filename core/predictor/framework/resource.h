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
#include <memory>
#include <string>
#include <vector>
#include "core/cube/cube-api/include/cube_api.h"
#include "core/predictor/common/inner_common.h"
#include "core/predictor/framework/infer.h"
#include "core/predictor/framework/memory.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

class PaddleGeneralModelConfig {
 public:
  PaddleGeneralModelConfig() {}

  ~PaddleGeneralModelConfig() {}

 public:
  std::vector<std::string> _feed_name;
  std::vector<std::string> _feed_alias_name;
  std::vector<int> _feed_type;      // 0 int64, 1 float
  std::vector<bool> _is_lod_feed;   // true lod tensor
  std::vector<bool> _is_lod_fetch;  // whether a fetch var is lod_tensor
  std::vector<int> _capacity;       //  capacity for each tensor
                                    /*
                                      feed_shape_ for feeded variable
                                      feed_shape_[i][j] represents the jth dim for ith input Tensor
                                      if is_lod_feed_[i] == False, feed_shape_[i][0] = -1
                                     */
  std::vector<std::vector<int>> _feed_shape;

  std::vector<std::string> _fetch_name;
  std::vector<std::string> _fetch_alias_name;
  std::vector<std::vector<int>> _fetch_shape;
  std::map<std::string, int> _fetch_name_to_index;
  std::map<std::string, int> _fetch_alias_name_to_index;
};

class BaseRdDict;
struct DynamicResource {
  DynamicResource();

  ~DynamicResource();

  int initialize();

  int clear();
};

class Resource {
 public:
  Resource() {
    // Reference InferManager::instance() explicitly, to make sure static
    // instance of InferManager is constructed before that of Resource, and
    // destruct after that of Resource
    // See https://stackoverflow.com/a/335746/1513460
    InferManager::instance();
  }

  ~Resource() { finalize(); }

  static Resource& instance() {
    static Resource ins;
    return ins;
  }

  int initialize(const std::string& path, const std::string& file);

  int general_model_initialize(const std::string& path,
                               const std::string& file);

  int thread_initialize();

  int thread_clear();

  int reload();

  int finalize();

  std::shared_ptr<PaddleGeneralModelConfig> get_general_model_config();

  void print_general_model_config(
      const std::shared_ptr<PaddleGeneralModelConfig>& config);

  DynamicResource* get_dynamic_resource() {
    return reinterpret_cast<DynamicResource*>(
        THREAD_GETSPECIFIC(_tls_bspec_key));
  }
  size_t get_cube_quant_bits();

 private:
  int thread_finalize() { return 0; }
  std::shared_ptr<PaddleGeneralModelConfig> _config;
  std::string cube_config_fullpath;
  int cube_quant_bits;  // 0 if no empty

  THREAD_KEY_T _tls_bspec_key;
};

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
