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
#include <stdexcept>

#include "core/preprocess/hwvideoframe/include/sub.h"
#include "core/preprocess/hwvideoframe/include/utils.h"

Sub::Sub(float subtractor) {
  for (size_t i = 0; i < CHANNEL_SIZE; i++) {
    _subtractors[i] = subtractor;
  }
}

Sub::Sub(const std::vector<float> &subtractors) {
  if (subtractors.size() != CHANNEL_SIZE) {
    throw std::runtime_error("size of subtractors must be three");
  }
  for (size_t i = 0; i < CHANNEL_SIZE; i++) {
    _subtractors[i] = subtractors[i];
  }
}

std::shared_ptr<OpContext> Sub::operator()(std::shared_ptr<OpContext> input) {
  NppStatus ret = nppiSubC_32f_C3IR(
      _subtractors, input->p_frame(), input->step(), input->nppi_size());
  verify_npp_ret("nppiSubC_32f_C3IR", ret);
  return input;
}
