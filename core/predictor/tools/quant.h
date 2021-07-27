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

#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include "core/predictor/framework.pb.h"
#include "seq_file.h"
using paddle::framework::proto::VarType;
void greedy_search(float* in,
                   float& xmin,
                   float& xmax,
                   float& loss,
                   size_t emb_size,
                   int bits);
// std::mutex g_mtx;

float compute_loss(float* a, float* b, int emb_size);
float* transfer(
    float* in, float* out, float min, float max, int emb_size, int bits);
char* quant(
    float* in, char** out, float min, float max, int emb_size, int bits);
float* dequant(
    char* in, float* out, float min, float max, int emb_size, int bits);
void greedy_search(float* in,
                   float& xmin,
                   float& xmax,
                   float& loss,
                   size_t emb_size,
                   int bits);
