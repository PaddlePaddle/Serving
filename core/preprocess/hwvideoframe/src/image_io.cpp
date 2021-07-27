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

#include <cuda_runtime.h>
#include <pybind11/numpy.h>

#include <stdexcept>

#include "core/preprocess/hwvideoframe/include/image_io.h"
#include "core/preprocess/hwvideoframe/include/utils.h"

std::shared_ptr<OpContext> Image2Gpubuffer::operator()(
    pybind11::array_t<float> input) {
  pybind11::buffer_info buf = input.request();
  if (buf.format != pybind11::format_descriptor<float>::format()) {
    throw std::runtime_error("Incompatible format: expected a float numpy!");
  }
  if (buf.ndim != 3) {
    throw std::runtime_error("Number of dimensions must be three");
  }
  if (buf.shape[2] != CHANNEL_SIZE) {
    throw std::runtime_error("Number of channels must be three");
  }
  auto result = std::make_shared<OpContext>(buf.shape[0], buf.shape[1]);
  auto ret = cudaMemcpy(result->p_frame(),
                        static_cast<float*>(buf.ptr),
                        result->size(),
                        cudaMemcpyHostToDevice);
  verify_cuda_ret("cudaMemcpy", ret);
  return result;
}

pybind11::array_t<float> Gpubuffer2Image::operator()(
    std::shared_ptr<OpContext> input) {
  auto result = pybind11::array_t<float>({input->nppi_size().height,
                                          input->nppi_size().width,
                                          static_cast<int>(CHANNEL_SIZE)});
  pybind11::buffer_info buf = result.request();
  auto ret = cudaMemcpy(static_cast<float*>(buf.ptr),
                        input->p_frame(),
                        input->size(),
                        cudaMemcpyDeviceToHost);
  verify_cuda_ret("cudaMemcpy", ret);
  return result;
}
