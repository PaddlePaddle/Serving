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

#include "predictor/framework/memory.h"
#include "predictor/common/inner_common.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

int MempoolWrapper::initialize() {
  if (THREAD_KEY_CREATE(&_bspec_key, NULL) != 0) {
    LOG(ERROR) << "unable to create thread_key of thrd_data";
    return -1;
  }
  if (THREAD_SETSPECIFIC(_bspec_key, NULL) != 0) {
    LOG(ERROR) << "failed initialize bsepecific key to null";
    return -1;
  }

  return 0;
}

int MempoolWrapper::thread_initialize() {
  _region.init();
  im::Mempool* p_mempool = new (std::nothrow) im::Mempool(&_region);
  if (p_mempool == NULL) {
    LOG(ERROR) << "Failed create thread mempool";
    return -1;
  }

  if (THREAD_SETSPECIFIC(_bspec_key, p_mempool) != 0) {
    LOG(ERROR) << "unable to set the thrd_data";
    delete p_mempool;
    return -1;
  }

  LOG(WARNING) << "Succ thread initialize mempool wrapper";
  return 0;
}

int MempoolWrapper::thread_clear() {
  im::Mempool* p_mempool = (im::Mempool*)THREAD_GETSPECIFIC(_bspec_key);
  if (p_mempool) {
    p_mempool->release_block();
    _region.reset();
  }

  return 0;
}

void* MempoolWrapper::malloc(size_t size) {
  im::Mempool* p_mempool = (im::Mempool*)THREAD_GETSPECIFIC(_bspec_key);
  if (!p_mempool) {
    LOG(WARNING) << "Cannot malloc memory:" << size
                 << ", since mempool is not thread initialized";
    return NULL;
  }
  return p_mempool->malloc(size);
}

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
