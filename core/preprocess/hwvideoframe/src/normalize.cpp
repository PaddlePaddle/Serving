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

#include "core/preprocess/hwvideoframe/include/normalize.h"
#include "core/preprocess/hwvideoframe/include/utils.h"

Normalize::Normalize(const std::vector<float> &mean,
                     const std::vector<float> &std,
                     bool channel_first) {
  if (mean.size() != CHANNEL_SIZE) {
    throw std::runtime_error("size of mean must be three");
  }
  if (std.size() != CHANNEL_SIZE) {
    throw std::runtime_error("size of std must be three");
  }
  for (size_t i = 0; i < CHANNEL_SIZE; i++) {
    _mean[i] = mean[i];
    _std[i] = std[i];
  }
  _channel_first = channel_first;
}

std::shared_ptr<OpContext> Normalize::operator()(
    std::shared_ptr<OpContext> input) {
  NppStatus ret = nppiSubC_32f_C3IR(
      _mean, input->p_frame(), input->step(), input->nppi_size());
  verify_npp_ret("nppiSubC_32f_C3IR", ret);
  ret = nppiDivC_32f_C3IR(
      _std, input->p_frame(), input->step(), input->nppi_size());
  verify_npp_ret("nppiDivC_32f_C3IR", ret);
  return input;
}
