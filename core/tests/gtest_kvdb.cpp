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
#include <functional>
#include <string>
#include <thread>
#include "core/kvdb/include/kvdb/kvdb_impl.h"
#include "core/kvdb/include/kvdb/paddle_rocksdb.h"
#include "core/kvdb/include/kvdb/rocksdb_impl.h"
class KVDBTest : public ::testing::Test {
 protected:
  void SetUp() override {}

  static void SetUpTestCase() {
    kvdb = std::make_shared<RocksKVDB>();
    dict_reader = std::make_shared<FileReader>();
    param_dict = std::make_shared<ParamDict>();
  }

  static AbsKVDBPtr kvdb;
  static FileReaderPtr dict_reader;
  static ParamDictPtr param_dict;
  static ParamDictMgr dict_mgr;
};
AbsKVDBPtr KVDBTest::kvdb;
FileReaderPtr KVDBTest::dict_reader;
ParamDictPtr KVDBTest::param_dict;
ParamDictMgr KVDBTest::dict_mgr;

void GenerateTestIn(std::string);
void UpdateTestIn(std::string);

TEST_F(KVDBTest, AbstractKVDB_Unit_Test) {
  kvdb->CreateDB();
  kvdb->SetDBName("test_kvdb");
  for (int i = 0; i < 100; i++) {
    kvdb->Set(std::to_string(i), std::to_string(i * 2));
  }
  for (int i = 0; i < 100; i++) {
    std::string val = kvdb->Get(std::to_string(i));
    ASSERT_EQ(val, std::to_string(i * 2));
  }
  kvdb->Close();
}

TEST_F(KVDBTest, FileReader_Unit_Test) {
  std::string test_in_filename = "abs_dict_reader_test_in.txt";
  GenerateTestIn(test_in_filename);
  dict_reader->SetFileName(test_in_filename);

  std::string md5_1 = dict_reader->GetMD5();
  std::chrono::system_clock::time_point timestamp_1 =
      dict_reader->GetTimeStamp();

  std::string md5_2 = dict_reader->GetMD5();
  std::chrono::system_clock::time_point timestamp_2 =
      dict_reader->GetTimeStamp();

  ASSERT_EQ(md5_1, md5_2);
  ASSERT_EQ(timestamp_1, timestamp_2);

  UpdateTestIn(test_in_filename);

  std::string md5_3 = dict_reader->GetMD5();
  std::chrono::system_clock::time_point timestamp_3 =
      dict_reader->GetTimeStamp();

  ASSERT_NE(md5_2, md5_3);
  ASSERT_NE(timestamp_2, timestamp_3);
}
#include <cmath>
void GenerateTestIn(std::string filename) {
  std::ifstream in_file(filename);
  if (in_file.good()) {
    in_file.close();
    std::string cmd = "rm -rf " + filename;
    system(cmd.c_str());
  }
  std::ofstream out_file(filename);
  for (size_t i = 0; i < 100000; i++) {
    out_file << i << " " << i << " ";
    for (size_t j = 0; j < 3; j++) {
      out_file << i << " ";
    }
    out_file << std::endl;
  }
  out_file.close();
}

void UpdateTestIn(std::string filename) {
  std::ifstream in_file(filename);
  if (in_file.good()) {
    in_file.close();
    std::string cmd = "rm -rf " + filename;
    system(cmd.c_str());
  }
  std::ofstream out_file(filename);
  for (size_t i = 0; i < 10000; i++) {
    out_file << i << " " << i << " ";
    for (size_t j = 0; j < 3; j++) {
      out_file << i + 1 << " ";
    }
    out_file << std::endl;
  }
  out_file.close();
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
