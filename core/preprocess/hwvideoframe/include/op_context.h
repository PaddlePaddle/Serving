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

#ifndef CORE_PREPROCESS_HWVIDEOFRAME_INCLUDE_OP_CONTEXT_H_
#define CORE_PREPROCESS_HWVIDEOFRAME_INCLUDE_OP_CONTEXT_H_

#include <npp.h>

const size_t CHANNEL_SIZE = 3;

// The context as input/ouput of all operators
// contains pointer to raw data on gpu, frame size
class OpContext {
 public:
  OpContext() {
    _step = 0;
    _size = 0;
    _p_frame = nullptr;
  }
  // constructor to apply gpu memory of image raw data
  OpContext(int height, int width) {
    _step = sizeof(Npp32f) * width * CHANNEL_SIZE;
    _length = height * width * CHANNEL_SIZE;
    _size = _step * height;
    _nppi_size.height = height;
    _nppi_size.width = width;
    cudaMalloc(reinterpret_cast<void**>(&_p_frame), _size);
  }
  virtual ~OpContext() { free_memory(); }

 public:
  Npp32f* p_frame() const { return _p_frame; }
  int step() const { return _step; }
  int length() const { return _length; }
  int size() const { return _size; }
  NppiSize& nppi_size() { return _nppi_size; }
  void free_memory() {
    if (_p_frame != nullptr) {
      cudaFree(_p_frame);
      _p_frame = nullptr;
    }
    _nppi_size.height = 0;
    _nppi_size.width = 0;
    _step = 0;
    _size = 0;
  }

 private:
  Npp32f* _p_frame;     // pointer to raw data on gpu
  int _step;            // number of bytes in a row
  int _length;          // length of _p_frame, _size = _length * sizeof(Npp32f)
  int _size;            // number of bytes of the image
  NppiSize _nppi_size;  // contains height and width
};

#endif  // CORE_PREPROCESS_HWVIDEOFRAME_INCLUDE_OP_CONTEXT_H_
