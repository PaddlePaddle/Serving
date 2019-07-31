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

namespace rec {
namespace mcube {

#define TIME_FLAG(flag) \
  struct timeval flag;  \
  gettimeofday(&(flag), NULL);

inline uint64_t time_diff(const struct timeval& start_time,
                          const struct timeval& end_time) {
  return (end_time.tv_sec - start_time.tv_sec) * 1000 +
         (end_time.tv_usec - start_time.tv_usec) / 1000;
}
}  // namespace mcube
}  // namespace rec
