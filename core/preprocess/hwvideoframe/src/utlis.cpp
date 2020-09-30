// Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
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

#include <npp.h>
#include <sstream>
#include <stdexcept>

#include "core/preprocess/hwvideoframe/include/utils.h"

void verify_npp_ret(const std::string& function_name, NppStatus ret) {
  if (ret != NPP_SUCCESS) {
    std::ostringstream ss;
    ss << function_name << ", ret: " << ret;
    throw std::runtime_error(ss.str());
  }
}

void verify_cuda_ret(const std::string& function_name, cudaError_t ret) {
  if (ret != cudaSuccess) {
    std::ostringstream ss;
    ss << function_name << ", ret: " << ret;
    throw std::runtime_error(ss.str());
  }
}
