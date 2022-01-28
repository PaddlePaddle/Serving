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

#include "core/predictor/framework/memory.h"
#include "core/predictor/common/inner_common.h"

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
  im::fugue::memory::Region* region = new im::fugue::memory::Region();
  region->init();
  im::Mempool* mempool = new (std::nothrow) im::Mempool(region);
  if (mempool == NULL) {
    LOG(ERROR) << "Failed create thread mempool";
    return -1;
  }
  MempoolRegion* mempool_region = new MempoolRegion(region, mempool);

  if (THREAD_SETSPECIFIC(_bspec_key, mempool_region) != 0) {
    LOG(ERROR) << "unable to set the thrd_data";
    delete region;
    delete mempool;
    delete mempool_region;
    return -1;
  }

  LOG(WARNING) << "Succ thread initialize mempool wrapper";
  return 0;
}

int MempoolWrapper::thread_clear() {
  MempoolRegion* mempool_region =
      (MempoolRegion*)THREAD_GETSPECIFIC(_bspec_key);
  if (mempool_region == NULL) {
    LOG(WARNING) << "THREAD_GETSPECIFIC() returned NULL";
    return -1;
  }
  im::Mempool* mempool = mempool_region->mempool();
  im::fugue::memory::Region* region = mempool_region->region();
  if (mempool) {
    mempool->release_block();
    region->reset();
  }
  return 0;
}

void* MempoolWrapper::malloc(size_t size) {
  MempoolRegion* mempool_region =
      (MempoolRegion*)THREAD_GETSPECIFIC(_bspec_key);
  if (mempool_region == NULL) {
    LOG(WARNING) << "THREAD_GETSPECIFIC() returned NULL";
    return NULL;
  }

  im::Mempool* mempool = mempool_region->mempool();
  if (!mempool) {
    LOG(WARNING) << "Cannot malloc memory:" << size
                 << ", since mempool is not thread initialized";
    return NULL;
  }
  return mempool->malloc(size);
}

void* MempoolWrapper::malloc(size_t size, MempoolRegion* my_mempool_region) {
  MempoolRegion* mempool_region = my_mempool_region;
  if (mempool_region == NULL) {
    LOG(WARNING) << "THREAD_GETSPECIFIC() returned NULL";
    return NULL;
  }

  im::Mempool* mempool = mempool_region->mempool();
  if (!mempool) {
    LOG(WARNING) << "Cannot malloc memory:" << size
                 << ", since mempool is not thread initialized";
    return NULL;
  }
  return mempool->malloc(size);
}

MempoolRegion* MempoolWrapper::get_thread_memory_ptr(){
  MempoolRegion* mempool_region =
      (MempoolRegion*)THREAD_GETSPECIFIC(_bspec_key);
  return mempool_region;
}

void MempoolWrapper::free(void* p, size_t size) {
  MempoolRegion* mempool_region =
      (MempoolRegion*)THREAD_GETSPECIFIC(_bspec_key);
  if (mempool_region == NULL) {
    LOG(WARNING) << "THREAD_GETSPECIFIC() returned NULL";
    return;
  }

  im::Mempool* mempool = mempool_region->mempool();
  if (!mempool) {
    LOG(WARNING) << "Cannot free memory:" << size
                 << ", since mempool is not thread initialized";
    return;
  }
  return mempool->free(p, size);
}

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
