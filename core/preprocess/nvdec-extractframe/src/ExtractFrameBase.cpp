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

#include "ExtractFrameBase.h"
#include <cmath>
int baidu::xvision::ExtractFrameBase::init() {
  ck(cuInit(0));
  int gpu_sum = 0;
  ck(cuDeviceGetCount(&gpu_sum));
  if (gpu_sum < 0 || gpu_sum < this->gpu_index) {
    return -1;
  }
  createCudaContext(&p_cu_context, gpu_index, 0);
  return 0;
}
bool baidu::xvision::ExtractFrameBase::select_frame(
    const double frame_rate,
    const int64_t pre_frame_time,
    const int64_t cur_frame_time,
    const size_t frame_index,
    const double fps,
    const int mode) {
  // TODO(Zelda): select frame function flattens ffmpeg FPS filter
  bool ret = false;
  int gap_time = 1000 / fps;
  int64_t pre_frame_timestamp_in_sec = pre_frame_time / gap_time;
  int64_t cur_frame_timestamp_in_sec = cur_frame_time / gap_time;
  int i_frame_rate = std::round(
      frame_rate);  // frame to int, 24.9999 will be 25, 24.02 will be 24
  int frame_gap = i_frame_rate / fps;  // frame gap, 24.99 will be 24
  if (frame_index == 0) {
    ret = true;
  }
  switch (mode) {
    case 0:  // recommended , same as ffmpeg fps filter, round:inf
      if (pre_frame_timestamp_in_sec != cur_frame_timestamp_in_sec) {
        ret = true;
      }
      break;
    case 1:
      if (frame_index % frame_gap == 0) {
        ret = true;
      }
      break;
    default:
      ret = false;
      break;
  }
  return ret;
}
