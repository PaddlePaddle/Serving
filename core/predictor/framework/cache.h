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
#include <sys/types.h>
#include <numeric>
#include <string>
#include <unordered_map>
#include "core/cube/cube-api/include/cube_api.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

// Large models that use sparse parameters may use cube cache.
// When the cube cache exists, the model is required to be
// consistent with the version of the cube cache. Therefore,
// when the model is updated, the model and the cube cache are
// required to be reloaded at the same time.
// Load all cached data at once without updating, it's lock free
// switching two cube cache.
class CubeCache {
 public:
  CubeCache() {}

  ~CubeCache() { clear(); }

  // clear cache data.
  int clear();

  // get cache data by key
  rec::mcube::CubeValue* get_data(uint64_t key);

  // reload all cache files from cache_path
  int reload_data(const std::string& cache_path);

 private:
  // switching free lock, key type is uint64_t, value type is CubeValue*
  std::unordered_map<uint64_t, rec::mcube::CubeValue*> _map_cache;
};

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
