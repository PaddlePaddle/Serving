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
#include <sstream>
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

std::vector<std::string> StringSplit(std::string str, char split) {
  std::vector<std::string> strs;
  std::istringstream f(str);
  std::string s;
  while (getline(f, s, split)) {
    strs.push_back(s);
  }
  return strs;
}

TEST_F(KVDBTest, AbstractKVDB_Func_Test) {
  AbsKVDBPtr kvdb = std::make_shared<RocksKVDB>();
  kvdb->CreateDB();
  std::string set_list = "setlist.txt";
  std::string get_list = "getlist.txt";
  std::ifstream set_file(set_list);
  std::ifstream get_file(get_list);
  for (std::string line; getline(set_file, line);) {
    std::vector<std::string> strs = StringSplit(line, ' ');
    kvdb->Set(strs[0], strs[1]);
  }

  for (std::string line; getline(get_file, line);) {
    std::vector<std::string> strs = StringSplit(line, ' ');
    std::string val = kvdb->Get(strs[0]);
    ASSERT_EQ(val, strs[1]);
  }
}

int main(int argc, char** argv) {
  my_argc = argc;
  my_argv = argv;
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
