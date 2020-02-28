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
#include "quant.h"
#include <cstring>
#include <string>
#include <cmath>
#include "ThreadPool.h"
#include "seq_file.h"
#include <mutex>
using paddle::framework::proto::VarType;
namespace baidu {
namespace paddle_serving {
namespace common {
float compute_loss(float* a, float* b, int emb_size) {
    float sum = 0;
    for (size_t i = 0; i < emb_size; i++) {
        sum += (a[i] - b[i]) * (a[i] - b[i]);
    }
    return sum;
}

float* transfer(float* in, float* out, float min, float max, int emb_size, int bits) {
    float scale = (max - min) / pow(2, bits);
    for(size_t i = 0; i < emb_size; i++) {
        float x = in[i];
        int val = round((x - min)/ (max - min) * (pow(2, bits) - 1));
        out[i] = val * scale + min;
    }
    return out;
}

char* quant(float* in, char* out, float min, float max, int emb_size, int bits) {
    float scale = (max - min) / pow (2, bits);
    for (size_t i = 0; i < emb_size; ++i) {
        float x= in[i];
        int val = round((x - min) / (max - min) * (pow(2, bits) - 1));
        out[emb_size] = (char)val;
    }
    return out;
}

float* dequant(char* in, float* out, float min, float max, int emb_size, int bits) {
    float scale = (max - min) / pow (2, bits);
    for (size_t i = 0; i < emb_size; ++i) {
        int val = in[i];
        float x = scale * val + min;
        out[i] = x;
    }
    return out;
}


void greedy_search(float* in, float &xmin, float &xmax, float& loss, size_t emb_size, int bits) {
    int b = 200;
    float r = 0.16;
    xmin = 2147483647;
    xmax = -2147483648;
    float cur_min = xmin;
    float cur_max = xmax;
    for(size_t i = 0; i< emb_size; i++) {
        xmin = std::min(xmin, in[i]);
        xmax = std::max(xmax, in[i]);
    }
    cur_min = xmin;
    cur_max = xmax;
    float out[emb_size];
    loss = compute_loss(in, transfer(in, out, cur_min, cur_max, emb_size, bits), emb_size);
    float stepsize = (cur_max - cur_min) / b;
    float min_steps = b * (1-r) * stepsize;
    while (cur_min + min_steps < cur_max) {
        float loss_l = compute_loss(in, transfer(in, out, cur_min + stepsize, cur_max, emb_size, bits), emb_size);
        float loss_r = compute_loss(in, transfer(in, out, cur_min, cur_max - stepsize, emb_size, bits), emb_size);
        if (loss_l < loss_r) {
            cur_min = cur_min + stepsize;
            if (loss_l < loss_r) {
                loss = loss_l; xmin = cur_min;
            }
        }
        else {
            cur_max = cur_max - stepsize;
            if (loss_r < loss) {
                loss = loss_r; xmax = cur_max;
            }
        }
    }
}

std::map<int, size_t> var_type_size;
std::mutex g_mtx;
void reg_var_types()
{
    var_type_size[static_cast<int>(VarType::FP16)] = sizeof(int16_t);
    var_type_size[static_cast<int>(VarType::FP32)] = sizeof(float);
    var_type_size[static_cast<int>(VarType::FP64)] = sizeof(double);
    var_type_size[static_cast<int>(VarType::INT32)] = sizeof(int);
    var_type_size[static_cast<int>(VarType::INT64)] = sizeof (int64_t);
    var_type_size[static_cast<int>(VarType::BOOL)] = sizeof(bool);
    var_type_size[static_cast<int>(VarType::SIZE_T)] = sizeof(size_t);
    var_type_size[static_cast<int>(VarType::INT16)] = sizeof(int16_t);
    var_type_size[static_cast<int>(VarType::UINT8)] = sizeof(uint8_t);
    var_type_size[static_cast<int>(VarType::INT8)] = sizeof(int8_t);
}

int writeSeq(const char* file1, const char* file2, int bits, int thread_pool_size) {

  std::cout << "make Sequence model for cube ..." << std::endl;
  std::ifstream is(file1); 
  // Step 1: is read version, os write version
  uint32_t version;
  is.read(reinterpret_cast<char *>(&version), sizeof(version));
  if (version != 0) {
    std::cout << "Version number " << version << " not supported" << std::endl;
    return -1;
  }
  std::cout << "Version size: " << sizeof(version) << std::endl;
  // Step 2: is read LoD level, os write LoD level
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
  // Step 3: is read Protobuf os Write Protobuf
  // Note: duplicate version field
  is.read(reinterpret_cast<char*>(&version), sizeof(version));
  if (version != 0) {
    std::cout << "Version number " << version << " not supported" << std::endl;
    return -1;
  }

  // Step 4: is read Tensor Data, os write  min/max/quant data
  VarType::TensorDesc desc;
  int32_t size;
  is.read(reinterpret_cast<char*>(&size), sizeof(size));
  std::unique_ptr<char[]> buf(new char[size]);
  is.read(reinterpret_cast<char*>(buf.get()), size);
  if (!desc.ParseFromArray(buf.get(), size)) {
    std::cout << "Cannot parse tensor desc" << std::endl;
    return -1;
  }
  // read tensor
  std::vector<int64_t> dims;
  dims.reserve(static_cast<size_t>(desc.dims().size()));
  std::copy(desc.dims().begin(), desc.dims().end(), std::back_inserter(dims));

  std::cout << "Dims:";
  for (auto x: dims) {
    std::cout << " " << x;
  }
  std::cout << std::endl;

  if (dims.size() != 2) {
    std::cout << "Parameter dims not 2D" << std::endl;
    return -1;
  }

  size_t numel = 1;
  for (auto x: dims) {
    numel *= x;
  }
  size_t buf_size = numel * var_type_size[desc.data_type()];
  std::cout << buf_size << std::endl;
  char* tensor_buf = new char[buf_size];
  is.read(static_cast<char*>(tensor_buf), buf_size);
  float* tensor_float_buf = reinterpret_cast<float*>(tensor_buf);
  size_t per_line_size = dims[1] * 1 + 2 * sizeof(float);

  float loss = 0;
  float all_loss = 0;
  std::cout << "Start Quant" << std::endl;
  SeqFileWriter seq_file_writer(file2);

  int value_buf_len =  per_line_size;
  char *value_buf = new char[value_buf_len];

  size_t offset = 0;
/*
  for (int64_t i = 0; i < dims[0]; ++i) {
    float xmin = 0, xmax = 0, loss = 0;
    size_t scale = dims[1];
    greedy_search(tensor_float_buf + i* dims[1], xmin, xmax, loss, scale, bits);
    quant(tensor_float_buf + i*dims[1], value_buf, xmin, xmax, dims[1], bits);
    memcpy(value_buf, tensor_buf + offset, value_buf_len);
    seq_file_writer.write((char *)&i, sizeof(i), value_buf, value_buf_len);
    offset += value_buf_len;
    std::cout << "dim: " << i << std::endl;
  }*/
  ThreadPool thread_pool(thread_pool_size);
  int thread_num = ceil((float)dims[0]/ 100000);
  auto parallel_seq = [&](int thread_id){
    for (int64_t i = thread_id * 100000; i < std::min((thread_id+1) * 100000, (int)dims[0]); ++i) {
      float xmin = 0, xmax = 0, loss = 0;
      size_t scale = dims[1];
      greedy_search(tensor_float_buf + i* dims[1], xmin, xmax, loss, scale, bits);
      quant(tensor_float_buf + i*dims[1], value_buf, xmin, xmax, dims[1], bits);
      std::cout << i << std::endl;
      //g_mtx.lock();
      memcpy(value_buf, tensor_buf + offset, value_buf_len);
      seq_file_writer.write((char *)&i, sizeof(i), value_buf, value_buf_len);
      //g_mtx.unlock();
      offset += value_buf_len;
    } 
  };

    
  for (int thread_id = 0; thread_id < thread_num; thread_id++) {
    thread_pool.enqueue(parallel_seq, thread_id);
  }
  return 0;
}
} // namespace common
} // namespace paddle_serving
} // namespace baidu
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
