
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
#include <vector>
#include <unordered_map>
#include <memory>
#include <chrono>
class AbstractKVDB;
class AbstractDictReader;
class AbstractParamDict;

typedef std::shared_ptr<AbstractKVDB> AbsKVDBPtr;
typedef std::shared_ptr<AbstractDictReader> AbsDictReaderPtr;
typedef std::shared_ptr<AbstractParamDict> AbsParamDictPtr;

class AbstractKVDB {
public:
    virtual void CreateDB() = 0;
    virtual void SetDBName(std::string) = 0;
    virtual void Set(std::string key, std::string value) = 0;
    virtual std::string Get(std::string key) = 0;
    inline virtual ~AbstractKVDB() = 0;
};

inline  AbstractKVDB::~AbstractKVDB() {}
// TODO: Implement RedisKVDB
//class RedisKVDB;

class AbstractDictReader {
public:
    virtual std::string GetFileName() = 0;
    virtual void SetFileName(std::string) = 0;
    virtual std::string GetMD5() = 0;
    virtual bool CheckDiff() = 0;
    virtual std::chrono::system_clock::time_point GetTimeStamp() = 0;
    virtual void Read(std::vector<std::string>&) = 0;
    virtual ~AbstractDictReader() = 0;
protected:
    std::string filename_;
    std::string last_md5_val_;
    std::chrono::system_clock::time_point time_stamp_;
};

inline  AbstractDictReader::~AbstractDictReader() {}

class AbstractParamDict {
public:
    virtual std::vector<AbsDictReaderPtr> GetDictReaderLst() = 0;
    virtual void SetDictReaderLst(std::vector<AbsDictReaderPtr>) = 0;

    virtual std::vector<float> GetSparseValue(int64_t, int64_t) = 0;
    virtual std::vector<float> GetSparseValue(std::string, std::string) = 0;
    
    virtual bool InsertSparseValue(int64_t, int64_t, const std::vector<float>&) = 0;
    virtual bool  InsertSparseValue(std::string, std::string, const std::vector<float>&) = 0;

    virtual void UpdateBaseModel() = 0;
    virtual void UpdateDeltaModel() = 0;

    virtual std::pair<AbsKVDBPtr, AbsKVDBPtr> GetKVDB() = 0;
    virtual void SetKVDB(std::pair<AbsKVDBPtr, AbsKVDBPtr>) = 0;
    virtual void CreateKVDB() = 0;

    virtual ~AbstractParamDict() = 0;
protected:
    std::vector<AbsDictReaderPtr> dict_reader_lst_;
    AbsKVDBPtr front_db, back_db;
};

inline  AbstractParamDict::~AbstractParamDict() {}


class ParamDictMgr {
public:
    void UpdateAll();
    void InsertParamDict(std::string, AbsParamDictPtr);

protected:
    std::unordered_map<std::string, AbsParamDictPtr> ParamDictMap;
};

