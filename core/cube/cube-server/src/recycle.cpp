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

#include "core/cube/cube-server/include/cube/recycle.h"

namespace {
using rec::mcube::Recycle;
static Recycle* g_instance = NULL;
}

namespace rec {
namespace mcube {

Recycle* Recycle::get_instance() {
  if (g_instance == NULL) {
    g_instance = new Recycle();
  }

  return g_instance;
}

Recycle::Recycle() : _running(false) {}

Recycle::~Recycle() {}

int Recycle::init() {
  // init mutex lock;
  if (pthread_mutex_init(&_recycle_mutex, NULL) != 0) {
    LOG(ERROR) << "init recycle lock failed";
    return -1;
  }

  _running = true;

  // init thread;
  if (pthread_create(&_recycle_thread,
                     NULL,
                     Recycle::recycle_func,
                     reinterpret_cast<void*>(this)) != 0) {
    LOG(ERROR) << "init recycle thread failed";
    return -1;
  }
  return 0;
}

int Recycle::destroy() {
  _running = false;
  // join thread;
  if (pthread_join(_recycle_thread, NULL) != 0) {
    LOG(WARNING) << "join recycle thread failed";
  }
  // destroy lock
  if (pthread_mutex_destroy(&_recycle_mutex) != 0) {
    LOG(WARNING) << "destroy recycle lock failed";
  }

  return 0;
}

/*
void Recycle::recycle(Dict* dict) {
    lock();
    _recycle_list.push(dict);
    unlock();
}
*/

void Recycle::recycle(VirtualDict* dict) {
  lock();
  _recycle_list.push(dict);
  unlock();
}

void Recycle::lock() { pthread_mutex_lock(&_recycle_mutex); }

void Recycle::unlock() { pthread_mutex_unlock(&_recycle_mutex); }

void* Recycle::recycle_func(void* arg) {
  Recycle* recycle = reinterpret_cast<Recycle*>(arg);
  std::queue<VirtualDict*>& recycle_list = recycle->_recycle_list;

  while (recycle->_running) {
    recycle->lock();
    if (recycle_list.empty()) {
      recycle->unlock();
      sleep(1);
      continue;
    }

    VirtualDict* dict = recycle_list.front();
    recycle_list.pop();
    recycle->unlock();

    while (dict->atom_seek_num() != 0) {
      sleep(1);
    }

    int ret = dict->destroy();
    if (ret != 0) {
      LOG(WARNING) << "destroy dict failed";
    }

    delete dict;
  }

  return NULL;
}

}  // namespace mcube
}  // namespace rec
