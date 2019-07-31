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
#include <memory>
#include <string>
#include "include/cube/cube_api.h"
#include "kvdb/paddle_rocksdb.h"
#include "predictor/common/inner_common.h"
#include "predictor/framework/memory.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

class BaseRdDict;
struct DynamicResource {
  DynamicResource();

  ~DynamicResource();

  int initialize();

  int clear();
};

class Resource {
 public:
  Resource() {}

  ~Resource() { finalize(); }

  static Resource& instance() {
    static Resource ins;
    return ins;
  }

  int initialize(const std::string& path, const std::string& file);
  int cube_initialize(const std::string& path, const std::string& file);
  int thread_initialize();

  int thread_clear();

  int reload();

  int finalize();

  std::shared_ptr<RocksDBWrapper> getDB();

  DynamicResource* get_dynamic_resource() {
    return reinterpret_cast<DynamicResource*>(
        THREAD_GETSPECIFIC(_tls_bspec_key));
  }

 private:
  int thread_finalize() { return 0; }
  std::shared_ptr<RocksDBWrapper> db;

  THREAD_KEY_T _tls_bspec_key;
};

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
