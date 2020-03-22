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

#include <algorithm>
#include <fstream>
#include <iterator>
#include <sstream>
#include <thread>  // NOLINT
#include "core/kvdb/include/kvdb/rocksdb_impl.h"

std::vector<FileReaderPtr> ParamDict::GetDictReaderLst() {
  return this->file_reader_lst_;
}

void ParamDict::SetFileReaderLst(std::vector<std::string> lst) {
  for (size_t i = 0; i < lst.size(); i++) {
    FileReaderPtr fr = std::make_shared<FileReader>();
    fr->SetFileName(lst[i]);
    this->file_reader_lst_.push_back(fr);
  }
}

std::vector<float> ParamDict::GetSparseValue(std::string feasign,
                                             std::string slot) {
  auto BytesToFloat = [](uint8_t* byte_array) {
    return *((float*)byte_array);  // NOLINT
  };
  // NOLINT TODO: the concatation of feasign and slot is TBD.
  std::string result = front_db->Get(feasign + slot);
  std::vector<float> value;
  if (result == "NOT_FOUND") return value;
  uint8_t* raw_values_ptr = reinterpret_cast<uint8_t*>(&result[0]);
  for (size_t i = 0; i < result.size(); i += sizeof(float)) {
    float temp = BytesToFloat(raw_values_ptr + i);
    value.push_back(temp);
  }
  return value;
}

void ParamDict::SetReader(
    std::function<std::pair<Key, Value>(std::string)> func) {
  read_func_ = func;
}

std::vector<float> ParamDict::GetSparseValue(int64_t feasign, int64_t slot) {
  return this->GetSparseValue(std::to_string(feasign), std::to_string(slot));
}

bool ParamDict::InsertSparseValue(int64_t feasign,
                                  int64_t slot,
                                  const std::vector<float>& values) {
  return this->InsertSparseValue(
      std::to_string(feasign), std::to_string(slot), values);
}

bool ParamDict::InsertSparseValue(std::string feasign,
                                  std::string slot,
                                  const std::vector<float>& values) {
  auto FloatToBytes = [](float fvalue, uint8_t* arr) {
    unsigned char* pf = nullptr;
    unsigned char* px = nullptr;
    unsigned char i = 0;
    pf = (unsigned char*)&fvalue;
    px = arr;
    for (i = 0; i < sizeof(float); i++) {
      *(px + i) = *(pf + i);
    }
  };

  std::string key = feasign + slot;
  uint8_t* values_ptr = new uint8_t[values.size() * sizeof(float)];
  std::string value;
  for (size_t i = 0; i < values.size(); i++) {
    FloatToBytes(values[i], values_ptr + sizeof(float) * i);
  }
  char* raw_values_ptr = reinterpret_cast<char*>(values_ptr);
  for (size_t i = 0; i < values.size() * sizeof(float); i++) {
    value.push_back(raw_values_ptr[i]);
  }
  back_db->Set(key, value);
  // NOLINT TODO: change stateless to stateful
  return true;
}

void ParamDict::UpdateBaseModel() {
  auto is_number = [](const std::string& s) {
    return !s.empty() && std::find_if(s.begin(), s.end(), [](char c) {
                           return !std::isdigit(c);
                         }) == s.end();
  };
  std::thread t([&]() {
    for (FileReaderPtr file_reader : this->file_reader_lst_) {
      std::string line;
      std::ifstream infile(file_reader->GetFileName());
      if (infile.is_open()) {
        while (getline(infile, line)) {
          std::pair<Key, Value> kvpair = read_func_(line);
          std::vector<float> nums;
          for (size_t i = 0; i < kvpair.second.size(); i++) {
            if (is_number(kvpair.second[i])) {
              nums.push_back(std::stof(kvpair.second[i]));
            }
          }
          this->InsertSparseValue(kvpair.first, "", nums);
        }
      }
      infile.close();
    }
    AbsKVDBPtr temp = front_db;
    front_db = back_db;
    back_db = temp;
  });
  t.detach();
}

void ParamDict::UpdateDeltaModel() { UpdateBaseModel(); }

std::pair<AbsKVDBPtr, AbsKVDBPtr> ParamDict::GetKVDB() {
  return {front_db, back_db};
}

void ParamDict::SetKVDB(std::pair<AbsKVDBPtr, AbsKVDBPtr> kvdbs) {
  this->front_db = kvdbs.first;
  this->back_db = kvdbs.second;
}

void ParamDict::CreateKVDB() {
  this->front_db = std::make_shared<RocksKVDB>();
  this->back_db = std::make_shared<RocksKVDB>();
  this->front_db->CreateDB();
  this->back_db->CreateDB();
}

ParamDict::~ParamDict() {}
