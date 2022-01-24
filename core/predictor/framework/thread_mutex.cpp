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

#include "core/predictor/framework/thread_mutex.h"
#include "core/predictor/common/inner_common.h"
namespace baidu {
namespace paddle_serving {
namespace predictor {

int ThreadMutex::initialize() {
  if (THREAD_KEY_CREATE(&_bspec_key_mutex, NULL) != 0) {
    LOG(ERROR) << "unable to create thread_key of thrd_data";
    return -1;
  }
  if (THREAD_SETSPECIFIC(_bspec_key_mutex, NULL) != 0) {
    LOG(ERROR) << "failed initialize bsepecific key to null";
    return -1;
  }

  if (THREAD_KEY_CREATE(&_bspec_key_cond, NULL) != 0) {
    LOG(ERROR) << "unable to create thread_key of thrd_data";
    return -1;
  }
  if (THREAD_SETSPECIFIC(_bspec_key_cond, NULL) != 0) {
    LOG(ERROR) << "failed initialize bsepecific key to null";
    return -1;
  }

  return 0;
}

int ThreadMutex::thread_initialize() {
  THREAD_MUTEX_T* mutex_ptr = new THREAD_MUTEX_T();
  THREAD_MUTEX_INIT(mutex_ptr, NULL);
  if (THREAD_SETSPECIFIC(_bspec_key_mutex, mutex_ptr) != 0) {
    LOG(ERROR) << "unable to set the thrd_data";
    delete mutex_ptr;
    return -1;
  }

  THREAD_COND_T* cont_ptr = new THREAD_COND_T();
  THREAD_COND_INIT(cont_ptr, NULL);
  if (THREAD_SETSPECIFIC(_bspec_key_cond, cont_ptr) != 0) {
    LOG(ERROR) << "unable to set the thrd_data";
    delete cont_ptr;
    return -1;
  }
  LOG(WARNING) << "Succ thread initialize ThreadMutex";
  return 0;
}

int ThreadMutex::thread_finalize() {
  THREAD_MUTEX_T* mutex_ptr =
      (THREAD_MUTEX_T*)THREAD_GETSPECIFIC(_bspec_key_mutex);
  if (mutex_ptr != NULL) {
    THREAD_MUTEX_DESTROY(mutex_ptr);
    delete mutex_ptr;
  }

  THREAD_COND_T* cont_ptr = (THREAD_COND_T*)THREAD_GETSPECIFIC(_bspec_key_cond);
  if (cont_ptr != NULL) {
    THREAD_COND_DESTROY(cont_ptr);
    delete cont_ptr;
  }
  LOG(WARNING) << "Succ thread initialize ThreadMutex";
  return 0;
}

int ThreadMutex::finalize() {
  THREAD_KEY_DELETE(_bspec_key_mutex);
  THREAD_KEY_DELETE(_bspec_key_cond);
  return 0;
}

THREAD_MUTEX_T* ThreadMutex::get_thread_mutex_ptr() {
  THREAD_MUTEX_T* mutex_ptr =
      (THREAD_MUTEX_T*)THREAD_GETSPECIFIC(_bspec_key_mutex);
  return mutex_ptr;
}

THREAD_COND_T* ThreadMutex::get_thread_cond_ptr() {
  THREAD_COND_T* cont_ptr = (THREAD_COND_T*)THREAD_GETSPECIFIC(_bspec_key_cond);
  return cont_ptr;
}

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
