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

#include <fstream>
#include <string>
#include "butil/logging.h"

class RawReader {
 public:
  RawReader() {}

  virtual ~RawReader() {}

  virtual int open() = 0;
  virtual int close() = 0;
  virtual int64_t read(int32_t* v) = 0;
  virtual int64_t read(uint32_t* v) = 0;
  virtual int64_t read(bool* v) = 0;
  virtual int64_t read(char* v) = 0;
  virtual int64_t read_buf(std::string* s, size_t len) = 0;
  virtual int64_t read_buf(void* v, size_t len) = 0;
  virtual const char* errno_to_str(int err) = 0;

 private:
};

class FileRawReader : public RawReader {
 public:
  explicit FileRawReader(const std::string& path) : _path(path) {}
  virtual ~FileRawReader() {}
  virtual int open() {
    _reader.open(_path.c_str(), std::ifstream::binary);
    if (!_reader.is_open()) {
      return -1;
    }
    LOG(INFO) << "raw open sequence file ok! file:" << _path.c_str();
    return 0;
  }
  virtual int close() {
    _reader.close();
    LOG(INFO) << "raw close sequence file ok! file:" << _path.c_str();
    return 0;
  }
  virtual int64_t read(int32_t* v) {
    _reader.read(reinterpret_cast<char*>(v), sizeof(int32_t));
    if (_reader.good())
      return sizeof(int32_t);
    else if (_reader.eof())
      return 0;
    else
      return -1;
  }
  virtual int64_t read(uint32_t* v) {
    _reader.read(reinterpret_cast<char*>(v), sizeof(uint32_t));
    if (_reader.good())
      return sizeof(uint32_t);
    else if (_reader.eof())
      return 0;
    else
      return -1;
  }
  virtual int64_t read(bool* v) {
    _reader.read(reinterpret_cast<char*>(v), sizeof(bool));
    if (_reader.good())
      return sizeof(bool);
    else if (_reader.eof())
      return 0;
    else
      return -1;
  }
  virtual int64_t read(char* v) {
    _reader.read(reinterpret_cast<char*>(v), sizeof(char));
    if (_reader.good())
      return sizeof(char);
    else if (_reader.eof())
      return 0;
    else
      return -1;
  }
  virtual int64_t read_buf(std::string* s, size_t len) {
    s->resize(len);
    std::string& tmp = *s;
    _reader.read(reinterpret_cast<char*>(&tmp[0]), len);
    if (_reader.good())
      return len;
    else if (_reader.eof())
      return 0;
    else
      return -1;
  }
  virtual int64_t read_buf(void* v, size_t len) {
    _reader.read(reinterpret_cast<char*>(v), len);
    if (_reader.good())
      return len;
    else if (_reader.eof())
      return 0;
    else
      return -1;
  }
  virtual const char* errno_to_str(int err) {
    switch (err) {
      case -1:
        return "read seqfile error";
    }
    return "default error";
  }

 private:
  std::string _path{""};
  std::ifstream _reader;
};
