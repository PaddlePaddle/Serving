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

#include <gtest/gtest.h>
#include <chrono>
#include <fstream>
#include <string>
#include <thread>
#include "core/kvdb/include/kvdb/kvdb_impl.h"
#include "core/kvdb/include/kvdb/paddle_rocksdb.h"
#include "core/kvdb/include/kvdb/rocksdb_impl.h"
class KVDBTest : public ::testing::Test {
 protected:
  void SetUp() override {}

  static void SetUpTestCase() {}
};

int my_argc;
char** my_argv;

void db_thread_test(AbsKVDBPtr kvdb, int size) {
  for (int i = 0; i < size; i++) {
    kvdb->Set(std::to_string(i), std::to_string(i));
    kvdb->Get(std::to_string(i));
  }
}

TEST_F(KVDBTest, AbstractKVDB_Thread_Test) {
  if (my_argc != 3) {
    std::cerr << "illegal input! should be db_thread ${num_of_thread} "
                 "${num_of_ops_each_thread}"
              << std::endl;
    return;
  }
  int num_of_thread = atoi(my_argv[1]);
  int nums_of_ops_each_thread = atoi(my_argv[2]);
  std::vector<AbsKVDBPtr> kvdbptrs;
  for (int i = 0; i < num_of_thread; i++) {
    kvdbptrs.push_back(std::make_shared<RocksKVDB>());
    kvdbptrs[i]->CreateDB();
  }
  std::vector<std::thread> tarr;
  for (int i = 0; i < num_of_thread; i++) {
    tarr.push_back(
        std::thread(db_thread_test, kvdbptrs[i], nums_of_ops_each_thread));
  }
  for (int i = 0; i < num_of_thread; i++) {
    tarr[i].join();
  }
  return;
}

int main(int argc, char** argv) {
  my_argc = argc;
  my_argv = argv;
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
