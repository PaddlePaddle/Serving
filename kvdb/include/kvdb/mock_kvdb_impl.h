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


#include "kvdb_impl.h"
#include "paddle_rocksdb.h"
class RocksKVDB: public AbstractKVDB {
public:
    void CreateDB();
    void SetDBName(std::string);
    void Set(std::string key, std::string value);
    std::string Get(std::string key);
    ~RocksKVDB();

protected:
    std::shared_ptr<RocksDBWrapper> db_;

public:
    static int db_count;
};

class MockDictReader : public AbstractDictReader{
public:
    std::string GetFileName();
    void SetFileName(std::string);
    std::string GetMD5();
    bool CheckDiff();
    std::chrono::system_clock::time_point GetTimeStamp();
    void Read(std::vector<std::string>&);
    ~MockDictReader();
};

class MockParamDict : public AbstractParamDict{
public:
    std::vector<AbsDictReaderPtr> GetDictReaderLst();
    void SetDictReaderLst(std::vector<AbsDictReaderPtr>);

    std::vector<float> GetSparseValue(int64_t, int64_t);
    std::vector<float> GetSparseValue(std::string, std::string);

    bool InsertSparseValue(int64_t, int64_t, const std::vector<float>&);
    bool InsertSparseValue(std::string, std::string, const std::vector<float>&);

    void UpdateBaseModel();
    void UpdateDeltaModel();

    std::pair<AbsKVDBPtr, AbsKVDBPtr> GetKVDB();
    void SetKVDB(std::pair<AbsKVDBPtr, AbsKVDBPtr>);
    void CreateKVDB();

    ~MockParamDict();
};


