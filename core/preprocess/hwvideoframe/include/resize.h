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

#ifndef CORE_PREPROCESS_HWVIDEOFRAME_INCLUDE_RESIZE_H_
#define CORE_PREPROCESS_HWVIDEOFRAME_INCLUDE_RESIZE_H_

#include <npp.h>
#include <memory>
#include <vector>

#include "core/preprocess/hwvideoframe/include/op_context.h"

extern "C" cudaError_t resize_linear(const float* input,
                                     float* output,
                                     const int inputWidth,
                                     const int inputHeight,
                                     const int outputWidth,
                                     const int outputHeight,
                                     const int inputChannels,
                                     const bool use_fixed_point);

// Resize the input numpy array Image to the given size.
// only support linear interpolation
// only support RGB channels
class Resize {
 public:
  // size is an int, smaller edge of the image will be matched to this number.
  Resize(int size,
         int max_size = 214748364,
         bool use_fixed_point = false,
         int interpolation = 0)
      : _size(size),
        _max_size(max_size),
        _use_fixed_point(use_fixed_point),
        _interpolation(interpolation) {}
  // size is a sequence like (w, h), output size will be matched to this
  Resize(std::vector<int> size,
         int max_size = 214748364,
         bool use_fixed_point = false,
         int interpolation = 0)
      : _size(-1),
        _max_size(max_size),
        _use_fixed_point(use_fixed_point),
        _interpolation(interpolation) {
    _target_size[0] = size[0];
    _target_size[1] = size[1];
  }
  std::shared_ptr<OpContext> operator()(std::shared_ptr<OpContext> input);

 private:
  int _size;            // target of smaller edge
  int _target_size[2];  // target size sequence (w, h)
  int _max_size;
  bool _use_fixed_point;
  int _interpolation;  // unused
};

#endif  // CORE_PREPROCESS_HWVIDEOFRAME_INCLUDE_RESIZE_H_
