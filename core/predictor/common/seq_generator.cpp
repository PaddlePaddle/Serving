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

#include <fstream>
#include <iostream>
#include <memory>
#include "core/predictor/framework.pb.h"
#include "seq_file.h"
using paddle::framework::proto::VarType;
std::map<int, size_t> var_type_size;
void reg_var_types() {
  var_type_size[static_cast<int>(VarType::FP16)] = sizeof(int16_t);
  var_type_size[static_cast<int>(VarType::FP32)] = sizeof(float);
  var_type_size[static_cast<int>(VarType::FP64)] = sizeof(double);
  var_type_size[static_cast<int>(VarType::INT32)] = sizeof(int);
  var_type_size[static_cast<int>(VarType::INT64)] = sizeof(int64_t);
  var_type_size[static_cast<int>(VarType::BOOL)] = sizeof(bool);
  var_type_size[static_cast<int>(VarType::SIZE_T)] = sizeof(size_t);
  var_type_size[static_cast<int>(VarType::INT16)] = sizeof(int16_t);
  var_type_size[static_cast<int>(VarType::UINT8)] = sizeof(uint8_t);
  var_type_size[static_cast<int>(VarType::INT8)] = sizeof(int8_t);
}
int dump_parameter(const char *input_file, const char *output_file) {
  std::ifstream is(input_file);
  // the 1st field, unit32_t version for LoDTensor
  uint32_t version;
  is.read(reinterpret_cast<char *>(&version), sizeof(version));
  if (version != 0) {
    std::cout << "Version number " << version << " not supported" << std::endl;
    return -1;
  }
  // the 2st field, LoD information
  uint64_t lod_level;
  is.read(reinterpret_cast<char *>(&lod_level), sizeof(lod_level));
  std::vector<std::vector<size_t>> lod;
  lod.resize(lod_level);
  for (uint64_t i = 0; i < lod_level; ++i) {
    uint64_t size;
    is.read(reinterpret_cast<char *>(&size), sizeof(size));
    std::vector<size_t> tmp(size / sizeof(size_t));
    is.read(reinterpret_cast<char *>(tmp.data()),
            static_cast<std::streamsize>(size));
    lod[i] = tmp;
  }
  // the 3st filed, Tensor
  // Note: duplicate version field
  is.read(reinterpret_cast<char *>(&version), sizeof(version));
  if (version != 0) {
    std::cout << "Version number " << version << " not supported" << std::endl;
    return -1;
  }
  // int32_t size
  // proto buffer
  VarType::TensorDesc desc;
  int32_t size;
  is.read(reinterpret_cast<char *>(&size), sizeof(size));
  std::unique_ptr<char[]> buf(new char[size]);
  is.read(reinterpret_cast<char *>(buf.get()), size);
  if (!desc.ParseFromArray(buf.get(), size)) {
    std::cout << "Cannot parse tensor desc" << std::endl;
    return -1;
  }
  // read tensor
  std::vector<int64_t> dims;
  dims.reserve(static_cast<size_t>(desc.dims().size()));
  std::copy(desc.dims().begin(), desc.dims().end(), std::back_inserter(dims));
  std::cout << "Dims:";
  for (auto x : dims) {
    std::cout << " " << x;
  }
  std::cout << std::endl;
  if (dims.size() != 2) {
    std::cout << "Parameter dims not 2D" << std::endl;
    return -1;
  }
  size_t numel = 1;
  for (auto x : dims) {
    numel *= x;
  }
  size_t buf_size = numel * var_type_size[desc.data_type()];
  char *tensor_buf = new char[buf_size];
  is.read(static_cast<char *>(tensor_buf), buf_size);
  is.close();
  SeqFileWriter seq_file_writer(output_file);
  int value_buf_len = var_type_size[desc.data_type()] * dims[1];
  char *value_buf = new char[value_buf_len];
  size_t offset = 0;
  for (int64_t i = 0; i < dims[0]; ++i) {
    // std::cout << "key_len " << key_len << " value_len " << value_buf_len <<
    // std::endl;
    memcpy(value_buf, tensor_buf + offset, value_buf_len);
    seq_file_writer.write((char *)&i, sizeof(i), value_buf, value_buf_len);
    offset += value_buf_len;
  }
  return 0;
}
int main(int argc, char **argv) {
  if (argc != 3) {
    std::cout << "Usage: seq_generator PARAMETER_FILE OUTPUT_FILE" << std::endl;
    return -1;
  }
  reg_var_types();
  dump_parameter(argv[1], argv[2]);
}
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
