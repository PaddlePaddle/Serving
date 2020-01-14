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
#include <queue>

#include "core/cube/cube-server/include/cube/dict.h"

namespace rec {
namespace mcube {

class Recycle {
 public:
  static Recycle* get_instance();

 public:
  ~Recycle();

  /**
   * @brief init recycle module
   */
  int init();

  /**
   * @brief destroy recycle module and wait recycle quit
   */
  int destroy();

  /**
   * @brief send dict to recycle module
   */
  // void recycle(Dict* dict);

  /**
   * @brief send dict to recycle module
   */
  void recycle(VirtualDict* dict);

 private:
  static void* recycle_func(void*);

  Recycle();

  /**
   * @brief lock recycle list
   */
  void lock();

  /**
   * @brief unlock recycle list
   */
  void unlock();

 private:
  pthread_t _recycle_thread;
  pthread_mutex_t _recycle_mutex;
  // std::queue<Dict*> _recycle_list;
  std::queue<VirtualDict*> _recycle_list;
  bool _running;
};  // class Recycle

}  // namespace mcube
}  // namespace rec
