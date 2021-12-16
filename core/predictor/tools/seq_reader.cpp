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
#include <limits.h> 
#include <fstream>
#include <iostream>
#include <memory>
#include <thread>
#include "core/cube/cube-builder/include/cube-builder/seqfile_reader.h"
std::string string_to_hex(const std::string& input) {
  static const char* const lut = "0123456789ABCDEF";
  size_t len = input.length();

  std::string output;
  output.reserve(2 * len);
  for (size_t i = 0; i < len; ++i) {
    const unsigned char c = input[i];
    output.push_back(lut[c >> 4]);
    output.push_back(lut[c & 15]);
  }
  return output;
}

void printSeq(std::string file, int limit) {
  SequenceFileRecordReader reader(file.c_str());

  if (reader.open() != 0) {
    std::cerr << "open file failed! " << file;
    return;
  }
  if (reader.read_header() != 0) {
    std::cerr << "read header error! " << file;
    reader.close();
    return;
  }

  Record record(reader.get_header());
  int total_count = 0;

  while (reader.next(&record) == 0) {
    uint64_t key =
        *reinterpret_cast<uint64_t *>(const_cast<char *>(record.key.data()));

    total_count++;
    int64_t value_length = record.record_len - record.key_len;
    float *data_ptr = new float[record.value.size() / 4];
    memcpy(data_ptr, record.value.data(), record.value.size()); 
    std::cout << "key: " << key << " , value: " << string_to_hex(record.value.c_str()) << std::endl;
    for (int i =0; i < record.value.size() / 4; ++i) {
      std::cout << data_ptr[i] << " ";
    } 
    std::cout << std::endl;
    delete(data_ptr);
    if (total_count >= limit) {
        break;
    }
  }

  if (reader.close() != 0) {
    std::cerr << "close file failed! " << file;
    return;
  }
}

int main(int argc, char **argv) {
    if (argc != 3 && argc != 2) {
        std::cout << "Seq Reader Usage:" << std::endl;
        std::cout << "get all keys: ./seq_reader $FILENAME " << std::endl;
        std::cout << "get some keys: ./seq_reader $FILENAME $KEY_NUM" << std::endl;
        return -1; 
    }
    if (argc == 3 || argc == 2) {
        const char* filename_str = argv[1];
        std::cout << "cstr filename is " << filename_str << std::endl;
        std::string filename = filename_str;
        std::cout << "filename is " << filename << std::endl;
        if (argc == 3) {
          const char* key_num_str = argv[2];
          int key_limit = std::stoi(key_num_str);
          printSeq(filename, key_limit);
        } else {
          printSeq(filename, INT_MAX);
        }
    }
    return 0;
}
