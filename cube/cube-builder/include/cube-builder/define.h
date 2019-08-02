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

#include <iostream>
#include <string>
#include <vector>
typedef std::string mode;

enum IndexType {
  IT_HASH,  ///< hash index
  IT_SORT,  ///< sort index
};

const int MAX_BLOCK_SIZE = 1024 * 1024 * 1024;

struct Meta {
  std::string key;
  std::string value;
};

struct Header {
  int version;
  std::string key_class;
  std::string value_class;
  bool is_compress;
  bool is_block_compress;
  std::string compress_class;
  std::vector<Meta> metas;
  std::string sync_marker;
  static const int s_sync_hash_size = 16;
};

struct Record {
  explicit Record(const Header& header) : sync_marker(header.sync_marker) {}
  const std::string& sync_marker;
  int record_len;
  int key_len;
  std::string key;
  std::string value;
};

class RecordReader {
 public:
  RecordReader() {}
  virtual ~RecordReader() {}
  virtual int open() = 0;

  virtual int next(Record* record) = 0;
  virtual int close() = 0;
};  // class RecordReader
