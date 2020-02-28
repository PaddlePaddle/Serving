// Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
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

#include <butil/files/temp_file.h>
#include <string>
#include <vector>
#include <random>
#include "core/predictor/unittest/test_quant.h"
#include "core/predictor/common/quant.h"
#include "core/predictor/unittest/test_tool.h"
#include <cstdlib>

namespace baidu {
namespace paddle_serving {
namespace unittest {

using namespace baidu::paddle_serving::common;

TEST_F(TestQuant, test_init) {
  float* in = new float[10];
  char* out = new char[2 * 4 + 10];
  for (size_t i = 0; i < 10; i++) {
    in[i] = std::rand();
  } 
  float min, max, loss;
  greedy_search(in, min, max, loss, 10, 8);
  quant(in, out, min, max, 10, 8);

}

TEST_F(TestQuant, test_seqfile) {
  //system("wget https://paddle-serving.bj.bcebos.com/unittest/SparseFeatFactors --no-check-certificate");
  int res = writeSeq("SparseFeatFactors", "SeqFile", 8, 1);
  ASSERT_EQ(res, 0);
}



} // namespace unittest
} // namespace paddle_serving
} // namespace baidu
