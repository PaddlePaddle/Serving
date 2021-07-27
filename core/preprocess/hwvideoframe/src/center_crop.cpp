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

#include <algorithm>

#include "core/preprocess/hwvideoframe/include/center_crop.h"
#include "core/preprocess/hwvideoframe/include/utils.h"

std::shared_ptr<OpContext> CenterCrop::operator()(
    std::shared_ptr<OpContext> input) {
  int new_width = std::min(_size, input->nppi_size().width);
  int new_height = std::min(_size, input->nppi_size().height);
  auto output = std::make_shared<OpContext>(new_height, new_width);
  int x_start = (input->nppi_size().width - new_width) / 2;
  int y_start = (input->nppi_size().height - new_height) / 2;
  Npp32f* p_src = input->p_frame() +
                  y_start * input->nppi_size().width * CHANNEL_SIZE +
                  x_start * CHANNEL_SIZE;
  NppStatus ret = nppiCopy_32f_C3R(p_src,
                                   input->step(),
                                   output->p_frame(),
                                   output->step(),
                                   output->nppi_size());
  verify_npp_ret("nppiCopy_32f_C3R", ret);
  return output;
}
