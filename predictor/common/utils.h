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
#include <string>
#include "predictor/common/inner_common.h"
#include "predictor/common/macros.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

class TimerFlow {
 public:
  static const int MAX_SIZE = 1024;

  TimerFlow() { init(); }

  void init() {
    _csize = 0;
    _name = NULL;
    _started = false;
    _auto = false;
  }

  explicit TimerFlow(const char* name) : _csize(0), _name(name) {
    _last = _start = butil::cpuwide_time_us();
    _auto = true;
    _started = true;
  }

  void set_name(const char* name) { _name = name; }

  void start() {
    _last = _start = butil::cpuwide_time_us();
    _started = true;
  }

  bool check(const char* tag) {
    if (!_started) {
      LOG(WARNING) << "Timer not started yet!";
      return false;
    }
    uint64_t now = butil::cpuwide_time_us();
    if (!appendf("%s:%lu|", tag, now - _last)) {
      LOG(WARNING) << "Failed check timer: " << _name << ", value = [" << tag
                   << ":" << (now - _last) << "]!";
      return false;
    }

    _last = now;
    return true;
  }

  std::string info() { return std::string(_buf); }

  void end() {
    uint64_t now = butil::cpuwide_time_us();
    if (!appendf("total:%lu", now - _start)) {
      LOG(WARNING) << "Failed dump time_info[" << _name << "]";
    }
    _started = false;
  }

  ~TimerFlow() {
    if (!_auto) {
      return;
    }
    uint64_t now = butil::cpuwide_time_us();
    if (appendf("total:%lu,%s", now - _start, _name)) {
      LOG(INFO) << " " << _name << "_tc=[" << _buf << "]";
    } else {
      LOG(WARNING) << "Failed dump time_info[" << _name << "]";
    }
  }

 private:
  bool appendf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    try {
      int bytes = vsnprintf(_buf + _csize, MAX_SIZE - _csize, fmt, ap);
      if (bytes >= MAX_SIZE - _csize || bytes < 0) {
        LOG(WARNING) << "Overflow when appendf!";
        return false;
      }
      _csize += bytes;
    }
    CATCH_ANY_AND_RET(false);

    va_end(ap);
    return true;
  }

 private:
  char _buf[1024];
  int _csize;
  uint64_t _start;
  uint64_t _last;
  const char* _name;
  bool _started;
  bool _auto;
};

template <bool flag>
struct derived_from_message {};

template <typename T, typename TBase>
class TIsDerivedFromB {
 private:
  static uint8_t check(TBase*) { return 1; }

  static uint32_t check(void*) { return 0; }

 public:
  enum {
    // function call cannot apprear in a constant-expression
    RESULT = (sizeof(uint8_t) == sizeof(check(reinterpret_cast<T*>(NULL)))),
  };
};

template <typename TBase>
class IsDerivedFrom {
 private:
  static bool check(TBase*) { return true; }

  static bool check(void*) { return false; }

 public:
  template <typename T>
  static bool yes(T* x) {
    return check(x);
  }
};

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
