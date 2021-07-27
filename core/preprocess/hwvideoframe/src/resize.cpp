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

#include "core/preprocess/hwvideoframe/include/resize.h"

#include <math.h>
#include <sstream>

#include "core/preprocess/hwvideoframe/include/utils.h"

std::shared_ptr<OpContext> Resize::operator()(
    std::shared_ptr<OpContext> input) {
  int resized_width = 0, resized_height = 0;
  if (_size == -1) {
    resized_width = std::min(_target_size[0], _max_size);
    resized_height = std::min(_target_size[1], _max_size);
  } else {
    int im_max_size =
        std::max(input->nppi_size().height, input->nppi_size().width);
    float percent =
        static_cast<float>(_size) /
        std::min(input->nppi_size().height, input->nppi_size().width);
    if (round(percent * im_max_size) > _max_size) {
      percent = static_cast<float>(_max_size) / static_cast<float>(im_max_size);
    }
    resized_width = static_cast<int>(round(input->nppi_size().width * percent));
    resized_height =
        static_cast<int>(round(input->nppi_size().height * percent));
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
