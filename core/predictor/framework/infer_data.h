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

#pragma once
#include <string>
#include <vector>
#include "core/predictor/common/inner_common.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

enum DataType { FLOAT32, INT64, INT32 };

class DataBuf {
 public:
  DataBuf() : _data(NULL), _size(0), _owned(true) {}

  explicit DataBuf(size_t size)
      : _data(new char[size]), _size(size), _owned(true) {}

  DataBuf(void* data, size_t size) : _data(data), _size(size), _owned(false) {}

  DataBuf(void* data, size_t size, bool owned)
      : _data(data), _size(size), _owned(owned) {}

  void* data() const { return _data; }

  size_t size() const { return _size; }

  void free() {
    _size = 0;
    if (_owned) {
      delete[](reinterpret_cast<char*>(_data));
    }
  }

  ~DataBuf() { free(); }

 private:
  void* _data;
  size_t _size;
  bool _owned;
};

struct Tensor {
  Tensor() {
    shape.clear();
    for (int li = 0; li < lod.size(); ++li) {
      lod[li].clear();
    }
    lod.clear();
  }

  Tensor(const Tensor& tensor) {
    name = tensor.name;
    data = tensor.data;
    type = tensor.type;
    shape.assign(tensor.shape.begin(), tensor.shape.end());
    for (int li = 0; li < tensor.lod.size(); ++li) {
      std::vector<size_t> l;
      l.assign(tensor.lod[li].begin(), tensor.lod[li].end());
      lod.push_back(l);
    }
  }

  ~Tensor() { shape.clear(); }

  size_t ele_byte() const {
    if (type == INT64) {
      return sizeof(int64_t);
    } else if (type == FLOAT32) {
      return sizeof(float);
    } else {
      return sizeof(int32_t);
    }
  }

  bool valid() const {
    if (shape.empty()) {
      if (data.data() || data.size()) {
        LOG(ERROR) << "data should be empty";
        return false;
      }
      return true;
    }

    if (!data.data() || !data.size()) {
      LOG(ERROR) << "data cannot empty";
      return false;
    }

    size_t byte_size = 1;
    for (size_t si = 0; si < shape.size(); ++si) {
      byte_size *= shape[si];
    }

    if (byte_size * ele_byte() != data.size()) {
      LOG(ERROR) << "wrong data size: " << byte_size * ele_byte() << " vs. "
                 << data.size();
      return false;
    }

    return true;
  }

  size_t shape0() {
    if (shape.empty()) {
      return 0;
    }
    return shape[0];
  }

  std::string name;
  std::vector<int> shape;
  DataBuf data;
  DataType type;
  std::vector<std::vector<size_t>> lod;
};

class BatchTensor {
 public:
  BatchTensor() {}
  ~BatchTensor() { _features.clear(); }

  BatchTensor(const BatchTensor& tv) {
    _features.assign(tv.features().begin(), tv.features().end());
  }

  Tensor& operator[](int index) { return _features[index]; }

  const Tensor& operator[](int index) const { return _features[index]; }

  void push_back(const Tensor& tensor) { _features.push_back(tensor); }

  size_t count() const { return _features.size(); }

  size_t size() const {
    // shape0 indicates batch_size
    if (count() <= 0 || _features[0].shape.size() <= 0) {
      return 0;
    }
    return _features[0].shape[0];
  }

  const std::vector<Tensor>& features() const { return _features; }

  void clear() { _features.clear(); }

 private:
  std::vector<Tensor> _features;
};

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
