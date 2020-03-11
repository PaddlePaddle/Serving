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
#include "rocksdb/compaction_filter.h"
#include "rocksdb/db.h"
#include "rocksdb/filter_policy.h"
#include "rocksdb/options.h"
#include "rocksdb/slice.h"
#include "rocksdb/sst_file_writer.h"
#include "rocksdb/table.h"

class RocksDBWrapper {
 public:
  RocksDBWrapper(std::string db_name);  // NOLINT
  std::string Get(std::string key);

  bool Put(std::string key, std::string value);
  void SetDBName(std::string db_name);
  static std::shared_ptr<RocksDBWrapper> RocksDBWrapperFactory(
      std::string db_name = "SparseMatrix");
  void Close();

 private:
  rocksdb::DB *db_;
  std::string db_name_;
};
