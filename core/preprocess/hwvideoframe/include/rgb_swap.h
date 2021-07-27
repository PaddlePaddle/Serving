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

#ifndef CORE_PREPROCESS_HWVIDEOFRAME_INCLUDE_RGB_SWAP_H_
#define CORE_PREPROCESS_HWVIDEOFRAME_INCLUDE_RGB_SWAP_H_

#include <npp.h>
#include <memory>
#include "core/preprocess/hwvideoframe/include/op_context.h"

// swap channel 0 and channel 2 for every pixel
// both RGB2BGR and BGR2RGB use this operator
class SwapChannel {
 public:
  SwapChannel() {}
  std::shared_ptr<OpContext> operator()(std::shared_ptr<OpContext> input);

 private:
  static const int
      _ORDER[CHANNEL_SIZE];  // describing how channel values are permutated
};

class RGB2BGR : public SwapChannel {};
class BGR2RGB : public SwapChannel {};

#endif  // CORE_PREPROCESS_HWVIDEOFRAME_INCLUDE_RGB_SWAP_H_
