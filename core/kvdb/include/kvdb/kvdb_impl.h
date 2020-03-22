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
#include <chrono>  // NOLINT
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>
class AbstractKVDB;
class FileReader;
class ParamDict;

typedef std::shared_ptr<AbstractKVDB> AbsKVDBPtr;
typedef std::shared_ptr<FileReader> FileReaderPtr;
typedef std::shared_ptr<ParamDict> ParamDictPtr;

class AbstractKVDB {
 public:
  virtual void CreateDB() = 0;
  virtual void SetDBName(std::string) = 0;
  virtual void Set(std::string key, std::string value) = 0;
  virtual std::string Get(std::string key) = 0;
  virtual void Close() = 0;
  virtual ~AbstractKVDB() = 0;
};

class FileReader {
 public:
  inline virtual std::string GetFileName() { return this->filename_; }

  inline virtual void SetFileName(std::string filename) {
    this->filename_ = filename;
    this->last_md5_val_ = this->GetMD5();
    this->time_stamp_ = std::chrono::system_clock::now();
  }

  inline virtual std::string GetMD5() {
    auto getCmdOut = [](std::string cmd) {
      std::string data;
      FILE *stream = nullptr;
      const int max_buffer = 256;
      char buffer[max_buffer];  // NOLINT
      cmd.append(" 2>&1");
      stream = popen(cmd.c_str(), "r");
      if (stream) {
        if (fgets(buffer, max_buffer, stream) != NULL) {
          data.append(buffer);
        }
      }
      return data;
    };
    std::string cmd = "md5sum " + this->filename_;
    // NOLINT TODO: throw exception if error occurs during execution of shell
    // command
    std::string md5val = getCmdOut(cmd);
    this->time_stamp_ = md5val == this->last_md5_val_
                            ? this->time_stamp_
                            : std::chrono::system_clock::now();
    this->last_md5_val_ = md5val;
    return md5val;
  }

  inline virtual bool CheckDiff() {
    return this->GetMD5() == this->last_md5_val_;
  }

  inline virtual std::chrono::system_clock::time_point GetTimeStamp() {
    return this->time_stamp_;
  }

  inline virtual ~FileReader() {}

 private:
  std::string filename_;
  std::string last_md5_val_;
  std::chrono::system_clock::time_point time_stamp_;
};

class ParamDict {
  typedef std::string Key;
  typedef std::vector<std::string> Value;

 public:
  virtual std::vector<FileReaderPtr> GetDictReaderLst();
  virtual void SetFileReaderLst(std::vector<std::string> lst);

  virtual std::vector<float> GetSparseValue(int64_t, int64_t);
  virtual std::vector<float> GetSparseValue(std::string, std::string);

  virtual bool InsertSparseValue(int64_t, int64_t, const std::vector<float> &);
  virtual bool InsertSparseValue(std::string,
                                 std::string,
                                 const std::vector<float> &);

  virtual void SetReader(std::function<std::pair<Key, Value>(std::string)>);
  virtual void UpdateBaseModel();
  virtual void UpdateDeltaModel();

  virtual std::pair<AbsKVDBPtr, AbsKVDBPtr> GetKVDB();
  virtual void SetKVDB(std::pair<AbsKVDBPtr, AbsKVDBPtr>);
  virtual void CreateKVDB();

  virtual ~ParamDict();

 private:
  std::function<std::pair<Key, Value>(std::string)> read_func_;  // NOLINT
  std::vector<FileReaderPtr> file_reader_lst_;
  AbsKVDBPtr front_db, back_db;
};

class ParamDictMgr {
 public:
  void UpdateAll();
  void InsertParamDict(std::string, ParamDictPtr);

 private:
  std::unordered_map<std::string, ParamDictPtr> ParamDictMap;  // NOLINT
};
