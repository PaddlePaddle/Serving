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

#include "kvdb/rocksdb_impl.h"
#include "kvdb/kvdb_impl.h"
#include "kvdb/paddle_rocksdb.h"
#include <gtest/gtest.h>
#include <string>
#include <fstream>
#include <chrono>
#include <thread>
class KVDBTest : public ::testing::Test {
protected:
    void SetUp() override{
                
    }
    
    static void SetUpTestCase() {
        kvdb = std::make_shared<RocksKVDB>();
        dict_reader = std::make_shared<RocksDBDictReader>();
        param_dict = std::make_shared<RocksDBParamDict>();
    }
    
    static AbsKVDBPtr kvdb;
    static AbsDictReaderPtr dict_reader;
    static AbsParamDictPtr param_dict;
    static ParamDictMgr dict_mgr;

};
AbsKVDBPtr KVDBTest::kvdb;
AbsDictReaderPtr KVDBTest::dict_reader;
AbsParamDictPtr KVDBTest::param_dict;
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
}

TEST_F(KVDBTest, AbstractDictReader_Unit_Test) {
    std::string test_in_filename = "abs_dict_reader_test_in.txt";
    GenerateTestIn(test_in_filename);
    dict_reader->SetFileName(test_in_filename);

    std::string md5_1 = dict_reader->GetMD5();
    std::chrono::system_clock::time_point timestamp_1 = dict_reader->GetTimeStamp();

    std::string md5_2 = dict_reader->GetMD5();
    std::chrono::system_clock::time_point timestamp_2 = dict_reader->GetTimeStamp();
    
    ASSERT_EQ(md5_1, md5_2);
    ASSERT_EQ(timestamp_1, timestamp_2);

    UpdateTestIn(test_in_filename);

    std::string md5_3 = dict_reader->GetMD5();
    std::chrono::system_clock::time_point timestamp_3 = dict_reader->GetTimeStamp();
    
    ASSERT_NE(md5_2, md5_3);
    ASSERT_NE(timestamp_2, timestamp_3);   
}
#include <cmath>
TEST_F(KVDBTest, RocksDBParamDict_Unit_Test) {
    std::string test_in_filename = "abs_dict_reader_test_in.txt";
    param_dict->SetDictReaderLst({dict_reader});
    param_dict->CreateKVDB();
    GenerateTestIn(test_in_filename);

    param_dict->UpdateBaseModel();

    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    std::vector<float> test_vec = param_dict->GetSparseValue(1, 1);

    ASSERT_LT(fabs(test_vec[0] - 1.0), 1e-2);

    UpdateTestIn(test_in_filename);
    param_dict->UpdateDeltaModel();
}

void GenerateTestIn(std::string filename) {
    std::ifstream in_file(filename);
    if (in_file.good()) {
        in_file.close();
        std::string cmd = "rm -rf "+ filename;
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

