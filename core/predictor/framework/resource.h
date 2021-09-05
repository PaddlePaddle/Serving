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
#include <unordered_map>
#include <utility>
#include <vector>

#include "core/cube/cube-api/include/cube_api.h"
#include "core/predictor/common/inner_common.h"
#include "core/predictor/framework/infer.h"
#include "core/predictor/framework/memory.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

// Paddle general model configuration, read the model configuration information
// from the general_model_config.proto file
class PaddleGeneralModelConfig {
 public:
  PaddleGeneralModelConfig() {}

  ~PaddleGeneralModelConfig() {}

 public:
  // feed/fetch name and alias_name
  std::vector<std::string> _feed_name;
  std::vector<std::string> _feed_alias_name;
  std::vector<std::string> _fetch_name;
  std::vector<std::string> _fetch_alias_name;

  // Be consistent with model saving interface var type conversion
  // (python/paddle serving client/io/__init__)
  // int64 => 0;
  // float32 => 1;
  // int32 => 2;
  // float64 => 3;
  // int16 => 4;
  // float16 => 5;
  // bfloat16 => 6;
  // uint8 => 7;
  // int8 => 8;
  // bool => 9;
  // complex64 => 10,
  // complex128 => 11;
  std::vector<int> _feed_type;

  // whether a feed or fetch var is lod_tensor.
  std::vector<bool> _is_lod_feed;
  std::vector<bool> _is_lod_fetch;

  // capacity for each tensor
  std::vector<int> _capacity;

  // _feed_shape and _fetch_shape are used to represent the dimensional
  // information of tensor.
  // for examples, feed_shape_[i][j] represents the j(th) dim for i(th) input
  // tensor.
  // if is_lod_feed_[i] == False, feed_shape_[i][0] = -1
  std::vector<std::vector<int>> _feed_shape;
  std::vector<std::vector<int>> _fetch_shape;

  // fetch name -> index of fetch_name vector.
  std::map<std::string, int> _fetch_name_to_index;

  // fetch alias name -> index of fetch_alias_name vector.
  std::map<std::string, int> _fetch_alias_name_to_index;
};

class BaseRdDict;

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

  // initialize resource
  int initialize(const std::string& path, const std::string& file);

  // loading all models configurations from prototxt
  int general_model_initialize(const std::string& path,
                               const std::string& file);

  // initialize thread local data
  int thread_initialize();

  // clear thread local data
  int thread_clear();

  // reload resources
  int reload();

  // finalize
  int finalize();

  // get all model configs
  std::vector<std::shared_ptr<PaddleGeneralModelConfig>>
  get_general_model_config();

  // print all configurations of all models
  void print_general_model_config(
      const std::shared_ptr<PaddleGeneralModelConfig>& config);

  // get cube quantity bit size
  size_t get_cube_quant_bits();

 private:
  int thread_finalize() { return 0; }

 private:
  // configuration infermation of all models, loading from prototxt files
  std::vector<std::shared_ptr<PaddleGeneralModelConfig>> _configs;

  // full path of cube configuration file.
  std::string _cube_config_fullpath;

  // cube quantify bit size, support 0/8. set 0 if no quant.
  size_t _cube_quant_bits;

  // bthread local key
  THREAD_KEY_T _tls_bspec_key;
};

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
