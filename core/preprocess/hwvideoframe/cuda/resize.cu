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

#include "./cuda_runtime.h"

#define clip(x, a, b) x >= a ? (x < b ? x : b - 1) : a;

const int INTER_RESIZE_COEF_BITS = 11;
const int INTER_RESIZE_COEF_SCALE = 1 << INTER_RESIZE_COEF_BITS;

__global__ void resizeCudaKernel(const float* input,
                                 float* output,
                                 const int inputWidth,
                                 const int inputHeight,
                                 const int outputWidth,
                                 const int outputHeight,
                                 const int inputChannels) {
  // 2D Index of current thread
  const int dx = blockIdx.x * blockDim.x + threadIdx.x;
  const int dy = blockIdx.y * blockDim.y + threadIdx.y;

  if ((dx < outputWidth) && (dy < outputHeight)) {
    if (inputChannels == 1) {  // grayscale image
      // TODO(Zelda): support grayscale
    } else if (inputChannels == 3) {  // RGB image
      double scale_x = static_cast<double>(inputWidth / outputWidth);
      double scale_y = static_cast<double>(inputHeight / outputHeight);

      int xmax = outputWidth;

      float fx = static_cast<float>((dx + 0.5) * scale_x - 0.5);
      int sx = floorf(fx);
      fx = fx - sx;

      int isx1 = sx;
      if (isx1 < 0) {
        fx = 0.0;
        isx1 = 0;
      }
      if (isx1 >= (inputWidth - 1)) {
        xmax = ::min(xmax, dx);
        fx = 0;
        isx1 = inputWidth - 1;
      }

      float2 cbufx;
      cbufx.x = (1.f - fx);
      cbufx.y = fx;

      float fy = static_cast<float>((dy + 0.5) * scale_y - 0.5);
      int sy = floorf(fy);
      fy = fy - sy;

      int isy1 = clip(sy + 0, 0, inputHeight);
      int isy2 = clip(sy + 1, 0, inputHeight);

      float2 cbufy;
      cbufy.x = (1.f - fy);
      cbufy.y = fy;

      int isx2 = isx1 + 1;

      float3 d0;

      float3 s11 =
          make_float3(input[(isy1 * inputWidth + isx1) * inputChannels + 0],
                      input[(isy1 * inputWidth + isx1) * inputChannels + 1],
                      input[(isy1 * inputWidth + isx1) * inputChannels + 2]);
      float3 s12 =
          make_float3(input[(isy1 * inputWidth + isx2) * inputChannels + 0],
                      input[(isy1 * inputWidth + isx2) * inputChannels + 1],
                      input[(isy1 * inputWidth + isx2) * inputChannels + 2]);
      float3 s21 =
          make_float3(input[(isy2 * inputWidth + isx1) * inputChannels + 0],
                      input[(isy2 * inputWidth + isx1) * inputChannels + 1],
                      input[(isy2 * inputWidth + isx1) * inputChannels + 2]);
      float3 s22 =
          make_float3(input[(isy2 * inputWidth + isx2) * inputChannels + 0],
                      input[(isy2 * inputWidth + isx2) * inputChannels + 1],
                      input[(isy2 * inputWidth + isx2) * inputChannels + 2]);

      float h_rst00, h_rst01;
      // B
      if (dx > xmax - 1) {
        h_rst00 = s11.x;
        h_rst01 = s21.x;
      } else {
        h_rst00 = s11.x * cbufx.x + s12.x * cbufx.y;
        h_rst01 = s21.x * cbufx.x + s22.x * cbufx.y;
      }
      d0.x = h_rst00 * cbufy.x + h_rst01 * cbufy.y;

      // G
      if (dx > xmax - 1) {
        h_rst00 = s11.y;
        h_rst01 = s21.y;
      } else {
        h_rst00 = s11.y * cbufx.x + s12.y * cbufx.y;
        h_rst01 = s21.y * cbufx.x + s22.y * cbufx.y;
      }
      d0.y = h_rst00 * cbufy.x + h_rst01 * cbufy.y;
      // R
      if (dx > xmax - 1) {
        h_rst00 = s11.z;
        h_rst01 = s21.z;
      } else {
        h_rst00 = s11.z * cbufx.x + s12.z * cbufx.y;
        h_rst01 = s21.z * cbufx.x + s22.z * cbufx.y;
      }
      d0.z = h_rst00 * cbufy.x + h_rst01 * cbufy.y;

      output[(dy * outputWidth + dx) * 3 + 0] = (d0.x);  // R
      output[(dy * outputWidth + dx) * 3 + 1] = (d0.y);  // G
      output[(dy * outputWidth + dx) * 3 + 2] = (d0.z);  // B
    } else {
      // TODO(Zelda): support alpha channel
    }
  }
}

__global__ void resizeCudaKernel_fixpt(const float* input,
                                       float* output,
                                       const int inputWidth,
                                       const int inputHeight,
                                       const int outputWidth,
                                       const int outputHeight,
                                       const int inputChannels) {
  // 2D Index of current thread
  const int dx = blockIdx.x * blockDim.x + threadIdx.x;
  const int dy = blockIdx.y * blockDim.y + threadIdx.y;

  if ((dx < outputWidth) && (dy < outputHeight)) {
    if (inputChannels == 1) {  // grayscale image
      // TODO(Zelda): support grayscale
    } else if (inputChannels == 3) {  // RGB image
      double scale_x = static_cast<double>(inputWidth / outputWidth);
      double scale_y = static_cast<double>(inputHeight / outputHeight);

      int xmax = outputWidth;

      float fx = static_cast<float>((dx + 0.5) * scale_x - 0.5);
      int sx = floorf(fx);
      fx = fx - sx;

      int isx1 = sx;
      if (isx1 < 0) {
        fx = 0.0;
        isx1 = 0;
      }
      if (isx1 >= (inputWidth - 1)) {
        xmax = ::min(xmax, dx);
        fx = 0;
        isx1 = inputWidth - 1;
      }

      short2 cbufx;
      cbufx.x = lrintf((1.f - fx) * INTER_RESIZE_COEF_SCALE);
      cbufx.y = lrintf(fx * INTER_RESIZE_COEF_SCALE);

      float fy = static_cast<float>((dy + 0.5) * scale_y - 0.5);
      int sy = floorf(fy);
      fy = fy - sy;

      int isy1 = clip(sy + 0, 0, inputHeight);
      int isy2 = clip(sy + 1, 0, inputHeight);

      short2 cbufy;
      cbufy.x = lrintf((1.f - fy) * INTER_RESIZE_COEF_SCALE);
      cbufy.y = lrintf(fy * INTER_RESIZE_COEF_SCALE);

      int isx2 = isx1 + 1;

      uchar3 d0;

      int3 s11 =
          make_int3(input[(isy1 * inputWidth + isx1) * inputChannels + 0],
                    input[(isy1 * inputWidth + isx1) * inputChannels + 1],
                    input[(isy1 * inputWidth + isx1) * inputChannels + 2]);
      int3 s12 =
          make_int3(input[(isy1 * inputWidth + isx2) * inputChannels + 0],
                    input[(isy1 * inputWidth + isx2) * inputChannels + 1],
                    input[(isy1 * inputWidth + isx2) * inputChannels + 2]);
      int3 s21 =
          make_int3(input[(isy2 * inputWidth + isx1) * inputChannels + 0],
                    input[(isy2 * inputWidth + isx1) * inputChannels + 1],
                    input[(isy2 * inputWidth + isx1) * inputChannels + 2]);
      int3 s22 =
          make_int3(input[(isy2 * inputWidth + isx2) * inputChannels + 0],
                    input[(isy2 * inputWidth + isx2) * inputChannels + 1],
                    input[(isy2 * inputWidth + isx2) * inputChannels + 2]);

      int h_rst00, h_rst01;
      // B
      if (dx > xmax - 1) {
        h_rst00 = s11.x * INTER_RESIZE_COEF_SCALE;
        h_rst01 = s21.x * INTER_RESIZE_COEF_SCALE;
      } else {
        h_rst00 = s11.x * cbufx.x + s12.x * cbufx.y;
        h_rst01 = s21.x * cbufx.x + s22.x * cbufx.y;
      }
      d0.x = (unsigned char)((((cbufy.x * (h_rst00 >> 4)) >> 16) +
                              ((cbufy.y * (h_rst01 >> 4)) >> 16) + 2) >>
                             2);

      // G
      if (dx > xmax - 1) {
        h_rst00 = s11.y * INTER_RESIZE_COEF_SCALE;
        h_rst01 = s21.y * INTER_RESIZE_COEF_SCALE;
      } else {
        h_rst00 = s11.y * cbufx.x + s12.y * cbufx.y;
        h_rst01 = s21.y * cbufx.x + s22.y * cbufx.y;
      }
      d0.y = (unsigned char)((((cbufy.x * (h_rst00 >> 4)) >> 16) +
                              ((cbufy.y * (h_rst01 >> 4)) >> 16) + 2) >>
                             2);
      // R
      if (dx > xmax - 1) {
        h_rst00 = s11.z * INTER_RESIZE_COEF_SCALE;
        h_rst01 = s21.z * INTER_RESIZE_COEF_SCALE;
      } else {
        h_rst00 = s11.z * cbufx.x + s12.z * cbufx.y;
        h_rst01 = s21.z * cbufx.x + s22.z * cbufx.y;
      }
      d0.z = (unsigned char)((((cbufy.x * (h_rst00 >> 4)) >> 16) +
                              ((cbufy.y * (h_rst01 >> 4)) >> 16) + 2) >>
                             2);

      output[(dy * outputWidth + dx) * 3 + 0] = (d0.x);  // R
      output[(dy * outputWidth + dx) * 3 + 1] = (d0.y);  // G
      output[(dy * outputWidth + dx) * 3 + 2] = (d0.z);  // B
    } else {
      // TODO(Zelda): support alpha channel
    }
  }
}

extern "C" cudaError_t resize_linear(const float* input,
                                     float* output,
                                     const int inputWidth,
                                     const int inputHeight,
                                     const int outputWidth,
                                     const int outputHeight,
                                     const int inputChannels,
                                     const bool use_fixed_point) {
  // Specify a reasonable block size
  const dim3 block(16, 16);

  // Calculate grid size to cover the whole image
  const dim3 grid((outputWidth + block.x - 1) / block.x,
                  (outputHeight + block.y - 1) / block.y);

  // Launch the size conversion kernel
  if (use_fixed_point) {
    resizeCudaKernel_fixpt<<<grid, block>>>(input,
                                            output,
                                            inputWidth,
                                            inputHeight,
                                            outputWidth,
                                            outputHeight,
                                            inputChannels);
  } else {
    resizeCudaKernel<<<grid, block>>>(input,
                                      output,
                                      inputWidth,
                                      inputHeight,
                                      outputWidth,
                                      outputHeight,
                                      inputChannels);
  }

  // Synchronize to check for any kernel launch errors
  return cudaDeviceSynchronize();
}
