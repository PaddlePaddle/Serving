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

#include <math.h>
#include <sstream>

#include "core/preprocess/hwvideoframe/include/resize.h"
#include "core/preprocess/hwvideoframe/include/resize_by_factor.h"
#include "core/preprocess/hwvideoframe/include/utils.h"

std::shared_ptr<OpContext> ResizeByFactor::operator()(
    std::shared_ptr<OpContext> input) {
  int resized_width = input->nppi_size().width,
      resized_height = input->nppi_size().height;
  float radio = 0;
  if (std::max(resized_width, resized_height) > _max_side_len) {
    if (resized_width > resized_height) {
      radio = static_cast<float>(_max_side_len / resized_width);
    } else {
      radio = static_cast<float>(_max_side_len / resized_height);
    }
  } else {
    radio = 1;
  }
  resized_width = static_cast<int>(resized_width * radio);
  resized_height = static_cast<int>(resized_height * radio);
  if (resized_height % _factor == 0) {
    resized_height = resized_height;
  } else if (floor(resized_height / _factor) <= 1) {
    resized_height = _factor;
  } else {
    resized_height = (floor(resized_height / 32) - 1) * 32;
  }
  if (resized_width % _factor == 0) {
    resized_width = resized_width;
  } else if (floor(resized_width / _factor) <= 1) {
    resized_width = _factor;
  } else {
    resized_width = (floor(resized_width / 32) - 1) * _factor;
  }
  if (static_cast<int>(resized_width) <= 0 ||
      static_cast<int>(resized_height) <= 0) {
    return NULL;
  }
  auto output = std::make_shared<OpContext>(resized_height, resized_width);
  auto ret = resize_linear(input->p_frame(),
                           output->p_frame(),
                           input->nppi_size().width,
                           input->nppi_size().height,
                           output->nppi_size().width,
                           output->nppi_size().height,
                           CHANNEL_SIZE,
                           _use_fixed_point);
  verify_cuda_ret("resize_linear", ret);
  return output;
}
