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

#include "quant.h"
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include "seq_file.h"
using paddle::framework::proto::VarType;

float compute_loss(float* a, float* b, int emb_size) {
  float sum = 0;
  for (size_t i = 0; i < emb_size; i++) {
    sum += (a[i] - b[i]) * (a[i] - b[i]);
  }
  return sum;
}

float* transfer(
    float* in, float* out, float min, float max, int emb_size, int bits) {
  float scale = (max - min) / pow(2, bits);
  for (size_t i = 0; i < emb_size; i++) {
    float x = in[i];
    int val = round((x - min) / (max - min) * (pow(2, bits) - 1));
    val = std::max(0, val);
    val = std::min((int)pow(2, bits) - 1, val);
    out[i] = val * scale + min;
  }
  return out;
}

char* quant(
    float* in, char** out, float min, float max, int emb_size, int bits) {
  float scale = (max - min) / pow(2, bits);
  for (size_t i = 0; i < emb_size; ++i) {
    float x = in[i];
    int val = round((x - min) / (max - min) * (pow(2, bits) - 1));
    val = std::max(0, val);
    val = std::min((int)pow(2, bits) - 1, val);
    *out[emb_size] = val;
  }
  return *out;
}

float* dequant(
    char* in, float* out, float min, float max, int emb_size, int bits) {
  float scale = (max - min) / pow(2, bits);
  for (size_t i = 0; i < emb_size; ++i) {
    float x =
        scale * (((int)in[i] + (int)pow(2, bits)) % (int)pow(2, bits)) + min;
    out[i] = x;
  }
  return out;
}

void greedy_search(float* in,
                   float& xmin,
                   float& xmax,
                   float& loss,
                   size_t emb_size,
                   int bits) {
  int b = 200;
  float r = 0.16;
  xmin = 2147483647;
  xmax = -2147483648;
  float cur_min = xmin;
  float cur_max = xmax;
  for (size_t i = 0; i < emb_size; i++) {
    xmin = std::min(xmin, in[i]);
    xmax = std::max(xmax, in[i]);
  }
  cur_min = xmin;
  cur_max = xmax;
  float out[emb_size];
  loss = compute_loss(
      in, transfer(in, out, cur_min, cur_max, emb_size, bits), emb_size);
  float stepsize = (cur_max - cur_min) / b;
  float min_steps = b * (1 - r) * stepsize;
  while (cur_min + min_steps < cur_max) {
    float loss_l = compute_loss(
        in,
        transfer(in, out, cur_min + stepsize, cur_max, emb_size, bits),
        emb_size);
    float loss_r = compute_loss(
        in,
        transfer(in, out, cur_min, cur_max - stepsize, emb_size, bits),
        emb_size);
    if (loss_l < loss) {
      cur_min = cur_min + stepsize;
      if (loss_l < loss_r) {
        loss = loss_l;
        xmin = cur_min;
      }
    } else {
      cur_max = cur_max - stepsize;
      if (loss_r < loss) {
        loss = loss_r;
        xmax = cur_max;
      }
    }
  }
}
