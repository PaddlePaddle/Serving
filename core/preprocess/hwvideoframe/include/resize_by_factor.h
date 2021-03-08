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

#ifndef CORE_PREPROCESS_HWVIDEOFRAME_INCLUDE_RESIZE_BY_FACTOR_H_
#define CORE_PREPROCESS_HWVIDEOFRAME_INCLUDE_RESIZE_BY_FACTOR_H_

#include <npp.h>

#include <memory>
#include <vector>

#include "core/preprocess/hwvideoframe/include/op_context.h"

extern "C" cudaError_t resize_linear(const float *input,
                                     float *output,
                                     const int inputWidth,
                                     const int inputHeight,
                                     const int outputWidth,
                                     const int outputHeight,
                                     const int inputChannels,
                                     const bool use_fixed_point);

// Resize the input numpy array Image to a size multiple of factor which is
// usually required by a network
// only support linear interpolation
// only support RGB channels
class ResizeByFactor {
 public:
  // Resize factor. make width and height multiple factor of the value of
  // factor. Default is 32
  ResizeByFactor(int factor = 32,
                 int max_side_len = 2400,
                 bool use_fixed_point = false,
                 int interpolation = 0)
      : _factor(factor),
        _max_side_len(max_side_len),
        _use_fixed_point(use_fixed_point),
        _interpolation(interpolation) {}
  std::shared_ptr<OpContext> operator()(std::shared_ptr<OpContext> input);

 private:
  int _factor;  // target of smaller edge
  int _max_side_len;
  bool _use_fixed_point;
  int _interpolation;  // unused
};

#endif  // CORE_PREPROCESS_HWVIDEOFRAME_INCLUDE_RESIZE_BY_FACTOR_H_
