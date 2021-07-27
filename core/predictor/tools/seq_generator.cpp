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

#include <sys/time.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <thread>

#include "core/predictor/framework.pb.h"
#include "quant.h"
#include "seq_file.h"

inline uint64_t time_diff(const struct timeval &start_time,
                          const struct timeval &end_time) {
  return (end_time.tv_sec - start_time.tv_sec) * 1000000 +
         (end_time.tv_usec - start_time.tv_usec);
}

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
    // std::cout << "key_len " << key_len << " value_len " << value_buf_len
    // << std::endl;
    memcpy(value_buf, tensor_buf + offset, value_buf_len);
    seq_file_writer.write((char *)&i, sizeof(i), value_buf, value_buf_len);
    offset += value_buf_len;
  }
  return 0;
}

float *read_embedding_table(const char *file1, std::vector<int64_t> &dims) {
  std::ifstream is(file1);
  // Step 1: is read version, os write version
  uint32_t version;
  is.read(reinterpret_cast<char *>(&version), sizeof(version));
  if (version != 0) {
    std::cout << "Version number " << version << " not supported" << std::endl;
    return NULL;
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
  is.read(reinterpret_cast<char *>(&version), sizeof(version));
  if (version != 0) {
    std::cout << "Version number " << version << " not supported" << std::endl;
    return NULL;
  }

  // Step 4: is read Tensor Data, os write  min/max/quant data
  VarType::TensorDesc desc;
  int32_t size;
  is.read(reinterpret_cast<char *>(&size), sizeof(size));
  std::unique_ptr<char[]> buf(new char[size]);
  is.read(reinterpret_cast<char *>(buf.get()), size);
  if (!desc.ParseFromArray(buf.get(), size)) {
    std::cout << "Cannot parse tensor desc" << std::endl;
    return NULL;
  }
  // read tensor
  // std::vector<int64_t> dims;
  dims.reserve(static_cast<size_t>(desc.dims().size()));
  std::copy(desc.dims().begin(), desc.dims().end(), std::back_inserter(dims));

  std::cout << "Dims:";
  for (auto x : dims) {
    std::cout << " " << x;
  }
  std::cout << std::endl;

  if (dims.size() != 2) {
    std::cout << "Parameter dims not 2D" << std::endl;
    return NULL;
  }

  size_t numel = 1;
  for (auto x : dims) {
    numel *= x;
  }
  size_t buf_size = numel * var_type_size[desc.data_type()];
  std::cout << buf_size << std::endl;
  char *tensor_buf = new char[buf_size];
  is.read(static_cast<char *>(tensor_buf), buf_size);
  float *tensor_float_buf = reinterpret_cast<float *>(tensor_buf);
  return tensor_float_buf;
}

int compress_parameter_parallel(const char *file1,
                                const char *file2,
                                int bits,
                                int n_threads) {
#define MIN_THREADS (1)
#define MAX_THREADS (80)
  std::vector<int64_t> dims;
  float *emb_table = read_embedding_table(file1, dims);
  if (emb_table == NULL || dims.size() != 2) {
    return -1;
  }
  // int64_t dict_size = dims[0]/100000000;
  int64_t dict_size = dims[0];
  int64_t emb_size = dims[1];
  size_t per_line_size = emb_size * 1 + 2 * sizeof(float);
  n_threads = std::min(std::max(MIN_THREADS, n_threads), MAX_THREADS);
  int64_t step = dict_size / n_threads;
  std::vector<char *> result;
  result.reserve(dict_size + 1);
  double pow2bits = pow(2, bits);
  std::cout << "Start Quant" << std::endl;
  std::vector<std::thread> threads;
  for (int i = 0; i < n_threads + 1; ++i) {
    threads.push_back(std::thread([=, &result]() {
      int64_t start = i * step;
      int64_t end = (i + 1) * step;
      if (i == n_threads) {
        if (start == dict_size) {
          return;
        }
        end = dict_size;
      }
      printf("THREAD[%d], index [%ld, %ld), start Quant table...\n",
             i,
             start,
             end);
      struct timeval quant_start;
      gettimeofday(&(quant_start), NULL);
      for (int64_t k = start; k < end; ++k) {
        float xmin = 0, xmax = 0, loss = 0;
        char *tensor_temp = new char[per_line_size];
        greedy_search(
            emb_table + k * emb_size, xmin, xmax, loss, emb_size, bits);
        // 得出 loss 最小的时候的 scale
        float scale = (xmax - xmin) / (pow2bits - 1);
        char *min_ptr = tensor_temp;
        char *max_ptr = tensor_temp + sizeof(float);
        memcpy(min_ptr, &xmin, sizeof(float));
        memcpy(max_ptr, &xmax, sizeof(float));
        for (size_t e = 0; e < emb_size; ++e) {
          float x = *(emb_table + k * emb_size + e);
          int val = round((x - xmin) / scale);
          val = std::max(0, val);
          val = std::min((int)pow2bits - 1, val);
          *(tensor_temp + 2 * sizeof(float) + e) = val;
        }
        result[k] = tensor_temp;
        if ((k - start) % 10000 == 0) {
          printf("THREAD[%d], handle line: %ld\n", i, k - start);
        }
      }
      struct timeval quant_end;
      gettimeofday(&(quant_end), NULL);
      printf("THREAD[%d], Quantization finished, cost: %lu us!!!\n",
             i,
             time_diff(quant_start, quant_end));
    }));
  }
  for (auto &thread : threads) {
    thread.join();
  }
  SeqFileWriter seq_file_writer(file2);
  for (int64_t i = 0; i < dict_size; i++) {
    seq_file_writer.write((char *)&i, sizeof(i), result[i], per_line_size);
  }
  return 0;
}
int main(int argc, char **argv) {
  if (argc < 3 || argc > 5) {
    std::cout << "Usage:" << std::endl;
    std::cout << "if no compress, please follow:" << std::endl;
    std::cout << "  seq_generator PARAMETER_FILE OUTPUT_FILE\n" << std::endl;
    std::cout << "if compress, please follow: " << std::endl;
    std::cout << "  seq_generator PARAMETER_FILE OUTPUT_FILE QUANT_BITS "
                 "[N_THREADS]"
              << std::endl;
    std::cout << "  Now it only support 8 bit." << std::endl;
    return -1;
  }
  reg_var_types();
  if (argc == 3) {
    std::cout << "generate normal sparse param sequence file" << std::endl;
    dump_parameter(argv[1], argv[2]);
    return 0;
  }
  if (argc == 4) {
    std::cout << "generate compressed sparse param sequence file" << std::endl;
    compress_parameter_parallel(argv[1], argv[2], atoi(argv[3]), 1);
    return 0;
  }
  if (argc == 5) {
    std::cout << "parallel generate compressed sparse param sequence file"
              << std::endl;
    compress_parameter_parallel(argv[1], argv[2], atoi(argv[3]), atoi(argv[4]));
    return 0;
  }
}
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
