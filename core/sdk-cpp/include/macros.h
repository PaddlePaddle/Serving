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
#include "core/sdk-cpp/include/common.h"

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

#ifndef CATCH_ANY_AND_RET
#define CATCH_ANY_AND_RET(errno)       \
  catch (...) {                        \
    LOG(ERROR) << "exception catched"; \
    return errno;                      \
  }
#endif

#define USE_PTHREAD

#ifdef USE_PTHREAD

#define THREAD_T pthread_t
#define THREAD_KEY_T pthread_key_t
#define THREAD_MUTEX_T pthread_mutex_t
#define THREAD_KEY_CREATE pthread_key_create
#define THREAD_SETSPECIFIC pthread_setspecific
#define THREAD_GETSPECIFIC pthread_getspecific
#define THREAD_CREATE pthread_create
#define THREAD_CANCEL pthread_cancel
#define THREAD_JOIN pthread_join
#define THREAD_KEY_DELETE pthread_key_delete
#define THREAD_MUTEX_INIT pthread_mutex_init
#define THREAD_MUTEX_LOCK pthread_mutex_lock
#define THREAD_MUTEX_UNLOCK pthread_mutex_unlock
#define THREAD_MUTEX_DESTROY pthread_mutex_destroy
#define THREAD_COND_T pthread_cond_t
#define THREAD_COND_INIT pthread_cond_init
#define THREAD_COND_SIGNAL pthread_cond_signal
#define THREAD_COND_WAIT pthread_cond_wait
#define THREAD_COND_DESTROY pthread_cond_destroy

#else

#define THREAD_T bthread_t
#define THREAD_KEY_T bthread_key_t
#define THREAD_MUTEX_T bthread_mutex_t
#define THREAD_KEY_CREATE bthread_key_create
#define THREAD_SETSPECIFIC bthread_setspecific
#define THREAD_GETSPECIFIC bthread_getspecific
#define THREAD_CREATE bthread_start_background
#define THREAD_CANCEL bthread_stop
#define THREAD_JOIN bthread_join
#define THREAD_KEY_DELETE bthread_key_delete
#define THREAD_MUTEX_INIT bthread_mutex_init
#define THREAD_MUTEX_LOCK bthread_mutex_lock
#define THREAD_MUTEX_UNLOCK bthread_mutex_unlock
#define THREAD_MUTEX_DESTROY bthread_mutex_destroy
#define THREAD_COND_T bthread_cond_t
#define THREAD_COND_INIT bthread_cond_init
#define THREAD_COND_SIGNAL bthread_cond_signal
#define THREAD_COND_WAIT bthread_cond_wait
#define THREAD_COND_DESTROY bthread_cond_destroy

#endif

}  // namespace sdk_cpp
}  // namespace paddle_serving
}  // namespace baidu
