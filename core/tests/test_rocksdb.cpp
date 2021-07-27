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

#include <iostream>
#include "core/kvdb/include/kvdb/paddle_rocksdb.h"
#include "core/kvdb/include/kvdb/rocksdb_impl.h"
void test_rockskvdb() {
  RocksKVDB db;
  db.CreateDB();
  db.SetDBName("Sparse Matrix");
  db.Set("1", "One");
  std::cout << db.Get("1") << std::endl;
  return;
}

void test_rocksdbwrapper() {
  std::shared_ptr<RocksDBWrapper> db =
      RocksDBWrapper::RocksDBWrapperFactory("TEST");
  for (size_t i = 0; i < 1000; i++) {
    db->Put(std::to_string(i), std::to_string(i * 2));
  }
  for (size_t i = 0; i < 1000; i++) {
    std::string res = db->Get(std::to_string(i));
    std::cout << res << " ";
  }
  std::cout << std::endl;
}

#ifdef RAW_TEST
int main() {
  test_rockskvdb();
  test_rocksdbwrapper();
}
#endif
