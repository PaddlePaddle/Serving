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

#include <memory>
#include <string>
#include "core/cube/cube-builder/include/cube-builder/define.h"
#include "core/cube/cube-builder/include/cube-builder/raw_reader.h"
class SequenceFileRecordReader : public RecordReader {
 public:
  SequenceFileRecordReader() {}
  explicit SequenceFileRecordReader(const std::string& path) {
    _path = path;
    _raw_reader = new FileRawReader(_path);
  }
  virtual ~SequenceFileRecordReader() {
    if (_raw_reader != nullptr) {
      delete _raw_reader;
    }
  }
  virtual int open();
  virtual int close();
  virtual int next(Record* record);
  const Header& get_header() { return _header; }
  int read_header();

 private:
  std::string _path{""};
  RawReader* _raw_reader{nullptr};
  Header _header;
};

typedef std::shared_ptr<SequenceFileRecordReader> SequenceFileRecordReaderPtr;
