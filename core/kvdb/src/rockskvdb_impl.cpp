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

#include "core/kvdb/include/kvdb/rocksdb_impl.h"

int RocksKVDB::db_count;
void RocksKVDB::CreateDB() {
  this->db_ = RocksDBWrapper::RocksDBWrapperFactory(
      "RocksDB_" + std::to_string(RocksKVDB::db_count));
  RocksKVDB::db_count++;
  return;
}

void RocksKVDB::SetDBName(std::string db_name) {
  this->db_->SetDBName(db_name);
  return;
}

void RocksKVDB::Set(std::string key, std::string value) {
  this->db_->Put(key, value);
  return;
}

void RocksKVDB::Close() { this->db_->Close(); }

std::string RocksKVDB::Get(std::string key) { return this->db_->Get(key); }

RocksKVDB::~RocksKVDB() { this->db_->Close(); }
