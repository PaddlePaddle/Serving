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
#include <fcntl.h>
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <string>

namespace baidu {
namespace paddle_serving {
namespace unittest {

class AutoTempFile {
 public:
  explicit AutoTempFile(const char* content) {
    _need_del = false;
    _name = generate_temp_name();
    FILE* fd = fopen(_name.c_str(), "w");
    if (!fd) {
      return;
    }
    fprintf(fd, "%s", content);
    fclose(fd);
    _need_del = true;
  }

  ~AutoTempFile() {
    if (_need_del) {
      remove(_name.c_str());
    }
  }

  const char* name() { return _name.c_str(); }

 private:
  std::string generate_temp_name() {
    timeval tv;
    srand(time(0));
    gettimeofday(&tv, NULL);
    std::ostringstream oss;
    oss << "uttest_temp_";
    oss << tv.tv_sec * 1000 + tv.tv_usec / 1000;
    oss << "_";
    oss << static_cast<int>(getpid());
    oss << "_";
    oss << rand_r();
    oss << ".conf";
    return oss.str();
  }

 private:
  std::string _name;
  bool _need_del;
};
}  // namespace unittest
}  // namespace paddle_serving
}  // namespace baidu
