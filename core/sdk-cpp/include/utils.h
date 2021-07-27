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
#include <vector>
#include "core/sdk-cpp/include/common.h"

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

inline int str_split(const std::string& source,
                     const std::string& delim,
                     std::vector<std::string>* vector_spliter) {
  int delim_length = delim.length();
  int total_length = source.length();
  int last = 0;

  if (delim_length == 0) {
    vector_spliter->push_back(source);
    return 0;
  }

  if (delim_length == 1) {
    size_t index = source.find_first_of(delim, last);
    while (index != std::string::npos) {
      vector_spliter->push_back(source.substr(last, index - last));
      last = index + delim_length;
      index = source.find_first_of(delim, last);
    }
  } else {
    size_t index = source.find(delim, last);
    while (index != std::string::npos) {
      vector_spliter->push_back(source.substr(last, index - last));
      last = index + delim_length;
      index = source.find(delim, last);
    }
  }

  if (last < total_length) {
    vector_spliter->push_back(source.substr(last, total_length - last));
  }
  return 0;
}

}  // namespace sdk_cpp
}  // namespace paddle_serving
}  // namespace baidu
