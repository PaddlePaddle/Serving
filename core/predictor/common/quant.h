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

#include <fstream>
#include <iostream>
#include <memory>
#include <cstring>
#include <string>
#include "core/predictor/framework.pb.h"
#include <cmath>
#include "ThreadPool.h"
#include "seq_file.h"
#include <mutex>

using paddle::framework::proto::VarType;
namespace baidu {
namespace paddle_serving{
namespace common {
void reg_var_types();
void greedy_search(float* in, float &xmin, float &xmax, float& loss, size_t emb_size, int bits);

float compute_loss(float* a, float* b, int emb_size);
float* transfer(float* in, float* out, float min, float max, int emb_size, int bits);
char* quant(float* in, char* out, float min, float max, int emb_size, int bits);
float* dequant(char* in, float* out, float min, float max, int emb_size, int bits);
void greedy_search(float* in, float &xmin, float &xmax, float& loss, size_t emb_size, int bits);
void reg_var_types();
int writeSeq(const char* file1, const char* file2, int bits, int thread_pool_size);
}
}
}
