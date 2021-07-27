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

#include <pthread.h>

namespace rec {
namespace mcube {

class RWLock {
 public:
  RWLock() { pthread_rwlock_init(&_lock, NULL); }

  ~RWLock() { pthread_rwlock_destroy(&_lock); }

  void r_lock() { pthread_rwlock_rdlock(&_lock); }

  void w_lock() { pthread_rwlock_wrlock(&_lock); }

  void unlock() { pthread_rwlock_unlock(&_lock); }

 private:
  pthread_rwlock_t _lock;
};  // class RWLock

}  // namespace mcube
}  // namespace rec
