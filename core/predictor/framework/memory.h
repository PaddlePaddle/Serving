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

#include "core/predictor/common/inner_common.h"
#include "core/predictor/mempool/mempool.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

// why we need MempoolRegion
// because we need to release the resource.
// so we need both Mempool and Region.
// Mempool is a wrapper class for us to use memory more safely.
// Region is the RAII class.
struct MempoolRegion {
  MempoolRegion(im::fugue::memory::Region* region, im::Mempool* mempool)
      : _region(region), _mempool(mempool) {}
  im::fugue::memory::Region* region() { return _region; }
  im::Mempool* mempool() { return _mempool; }

  im::fugue::memory::Region* _region;
  im::Mempool* _mempool;
  ~MempoolRegion() {
    if (_region) {
      delete _region;
      _region = NULL;
    }
    if (_mempool) {
      delete _mempool;
      _mempool = NULL;
    }
  }
};
class MempoolWrapper {
 public:
  MempoolWrapper() {}

  static MempoolWrapper& instance() {
    static MempoolWrapper mempool;
    return mempool;
  }

  int initialize();

  int thread_initialize();

  int thread_clear();

  void* malloc(size_t size);

  void* malloc(size_t size, MempoolRegion* my_mempool_region);

  MempoolRegion* get_thread_memory_ptr();

  void free(void* p, size_t size);

 private:
  // im::fugue::memory::Region _region;
  THREAD_KEY_T _bspec_key;
};

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
