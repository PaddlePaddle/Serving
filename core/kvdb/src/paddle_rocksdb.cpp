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

#include "core/kvdb/include/kvdb/paddle_rocksdb.h"

RocksDBWrapper::RocksDBWrapper(std::string db_name) {
  rocksdb::Options options;
  options.create_if_missing = true;
  db_name_ = db_name;
  db_ = nullptr;
  rocksdb::Status s = rocksdb::DB::Open(options, db_name, &db_);
  return;
}

std::string RocksDBWrapper::Get(std::string key) {
  rocksdb::ReadOptions options;
  options.verify_checksums = true;
  std::string result;
  rocksdb::Status s = db_->Get(options, key, &result);
  if (s.IsNotFound()) {
    result = "NOT_FOUND";
  }
  return result;
}

bool RocksDBWrapper::Put(std::string key, std::string value) {
  rocksdb::WriteOptions options;
  rocksdb::Status s = db_->Put(options, key, value);
  if (s.ok()) {
    return true;
  } else {
    return false;
  }
}

void RocksDBWrapper::SetDBName(std::string db_name) {
  this->db_name_ = db_name;
}

void RocksDBWrapper::Close() {
  if (db_ != nullptr) {
    db_->Close();
    delete (db_);
    db_ = nullptr;
  }
}

std::shared_ptr<RocksDBWrapper> RocksDBWrapper::RocksDBWrapperFactory(
    std::string db_name) {
  return std::make_shared<RocksDBWrapper>(db_name);
}
