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

#include "core/preprocess/hwvideoframe/include/div.h"
#include "core/preprocess/hwvideoframe/include/utils.h"

Div::Div(float value) { _divisor = value; }

std::shared_ptr<OpContext> Div::operator()(std::shared_ptr<OpContext> input) {
  NppStatus ret = nppsDivC_32f_I(_divisor, input->p_frame(), input->length());
  verify_npp_ret("nppsDivC_32f_I", ret);
  return input;
}
