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

#include <atomic>
#include <string>
#include <vector>
#include <unordered_map>
#include "butil/third_party/rapidjson/document.h"
#include "butil/third_party/rapidjson/prettywriter.h"
#include "butil/third_party/rapidjson/stringbuffer.h"
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <sstream>
#include "core/cube/cube-server/cube.pb.h"
#include "core/cube/cube-server/include/cube/dict.h"
#include "core/cube/cube-server/include/cube/rw_lock.h"
#include "core/cube/cube-server/include/cube/virtual_dict.h"

namespace rec {
namespace mcube {

struct Status {
  enum StatusCode { F_RUNNING = 0, F_LOADING };
};
struct Record {
  int32_t id;
  int32_t version;
  int64_t key;
  std::string path;
  std::string cmd;
  std::string timestamp;
};
class VersionTable {
public:
  VersionTable(std::string dict_name) {
    _record_file = "./data/" + dict_name+ "/version.txt";
    _cur_version = 0;
    _cur_id = 0;
  }
  std::vector<Record> load_records() {
    std::vector<Record> records;
    std::ifstream fs;
    LOG(INFO) << "Versiontable read records";
    fs.open(this->_record_file);
    while (!fs.eof()) {
      char cstr[100];
      fs.getline(cstr, 100);
      std::string str(cstr);
      LOG(INFO) << "load record: " << str;
      std::vector<std::string> result;
      std::stringstream ss(str);
      while(ss.good())
      {
        std::string substr;
        getline(ss, substr, '|');
        result.push_back(substr);
      }
      if (result.size() == 6) {
        Record rec;
        rec.id = stoi(result[0]);
        rec.version = stoi(result[1]);
        rec.key = stol(result[2]);
        rec.path = result[3];
        rec.cmd = result[4];
        rec.timestamp = result[5];
        records.push_back(rec);
      LOG(INFO) << "finish load record: " << str;
      }
    } 
    _records = records;
    return records;     
  }
  void save_records() {
    std::ofstream fs;
    fs.open(this->_record_file, std::ios::ate|std::ios::out);
    std::string content = "";
    for (const Record& rec: this->_records) {
      LOG(INFO) << "save record: " << std::to_string(rec.id) << ", " << std::to_string(rec.version) << ", " << std::to_string(rec.key) << ", " <<rec.path << ", " << rec.cmd; 
      content += std::to_string(rec.id)+ "|";
      content += std::to_string(rec.version)+ "|";
      content += std::to_string(rec.key) + "|";
      content += rec.path + "|";
      content += rec.cmd + "|";
      content += rec.timestamp + "\n";
      fs.write(content.c_str(), content.size());
    }
    fs.close();
  }
  void add_record(int64_t key, std::string path, std::string cmd) {
    std::ofstream fs;
    fs.open(this->_record_file, std::ios_base::app);
    std::string content = "";
    int32_t this_id = ++_cur_id;
    int32_t this_version = ++_cur_version;
    int64_t this_key = key;
    std::string this_cmd = cmd;
    std::string this_timestamp = get_ts();
    std::string this_path = path;
    Record rec;
    rec.id = this_id;
    rec.version = this_version;
    rec.key = this_key;
    rec.path = this_path;
    rec.cmd = this_cmd;
    rec.timestamp = this_timestamp;
    _records.push_back(rec);
    content += std::to_string(rec.id)+ "|";
    content += std::to_string(rec.version)+ "|";
    content += std::to_string(rec.key) + "|";
    content += rec.path + "|";
    content += rec.cmd + "|";
    content += rec.timestamp + "\n";
    fs.write(content.c_str(), content.size());
    fs.close();
  }
  std::string to_string() {
    std::string content = "";
    for (const Record& rec: this->_records) {
      content += std::to_string(rec.id)+ "|";
      content += std::to_string(rec.version)+ "|";
      content += std::to_string(rec.key) + "|";
      content += rec.path + "|";
      content += rec.cmd + "|";
      content += rec.timestamp + "\n";
    }
    return content;
  }
private:
  std::string get_ts() {
    char date[20];
    time_t t = time(0);
    struct tm *tm;

    tm = gmtime(&t);
    strftime(date, sizeof(date), "%Y%m%dT%T", tm);
    std::string ss(date);
    return ss;
  }
  std::string _record_file;
  std::vector<Record> _records;
  int32_t _cur_version;
  int32_t _cur_id;
};

class DoubleBufDict 
{
 public:
  void init_dict(std::string dict_name) {
    _dict[0] = new (std::nothrow) Dict();
    _dict[1] = nullptr;
    this->_version_table = new VersionTable(dict_name);
    this->_dict_name = dict_name;
  }

  VirtualDict* get_dict(int idx) {
    return _dict[idx];
  }
  void set_dict(VirtualDict* dict, int idx) {
    _dict[idx] = dict;
  }
  VirtualDict* get_cur_dict() {
    _rw_lock.r_lock();
    VirtualDict* dict = get_dict(_dict_idx);
    dict->atom_inc_seek_num();
    _rw_lock.unlock();
    return dict;
  }
  std::string get_cur_version() {
    _rw_lock.r_lock();
    VirtualDict* dict = get_cur_dict();
    std::string version = dict->version();
    _rw_lock.unlock();
    return version;
  }
  VirtualDict* get_bg_dict() {
    return _dict[1 - _dict_idx]; 
  }
  
  std::string get_bg_version() {
    _bg_rw_lock.r_lock();
    VirtualDict* dict = _dict[1 - _dict_idx];
    std::string version = "";
    if (dict) {
      version = dict->guard_version();
    }
    _bg_rw_lock.unlock();
    return version;
  }
  
  void set_bg_dict(VirtualDict* dict) {
    _bg_rw_lock.w_lock();
    _dict[1 - _dict_idx] = dict;
    _bg_rw_lock.unlock();
  }
  
  int bg_switch() {
   _rw_lock.w_lock();
   int bg_idx = 1 - _dict_idx;
   if (!_dict[bg_idx]) {
     LOG(WARNING) << "switch dict failed because nullptr";
     _rw_lock.unlock();
     return -1;
   }
   _dict_idx = bg_idx;
   _rw_lock.unlock();
   return 0;
  }
  
  void set_dict_path(std::string dict_path) {
    _dict_path = dict_path;
  }  

  std::string get_dict_path() {
    return _dict_path;
  }

  void set_in_mem(bool in_mem) {
    _in_mem = in_mem;
  }
  bool get_in_mem() {
    return _in_mem;
  }
  VersionTable* _version_table;

 private:
  VirtualDict* _dict[2]{nullptr, nullptr};
  int _dict_idx;
  bool _in_mem;
  uint32_t _max_val_size;
  RWLock _rw_lock;
  RWLock _bg_rw_lock;
  std::string _dict_path;
  std::string _dict_name;
};


class Framework {
 public:
  static Framework* instance();

 public:
  ~Framework();

  //int init(std::string dict_name, uint32_t dict_split, bool in_mem);
  int init();

  int destroy();

  int status(BUTIL_RAPIDJSON_NAMESPACE::Document* res, std::string dict_name);

  int seek(const DictRequest* req, DictResponse* res);

  int reload(std::string dict_name, const std::string& v_path);

  int patch(std::string dict_name, const std::string& v_path);

  // load dict base
  int bg_load_base(std::string dict_name, const std::string& v_path);

  // load dict patch
  int bg_load_patch(std::string dict_name, const std::string& v_path);

  int bg_unload(std::string dict_name);

  int bg_switch(std::string dict_name);

  int enable(std::string dict_name, const std::string& version);

 private:
  void init_dict(std::string dict_name, uint32_t dict_split);

  VirtualDict* create_dict();

 private:
  VirtualDict* get_cur_dict(std::string dict_name) {
    if (_dict_map.find(dict_name) != _dict_map.end()) {
      DoubleBufDict* ddict = _dict_map.at(dict_name);
      return ddict->get_cur_dict();
    } else {
      return nullptr;
    }
  }

  std::string get_cur_version(std::string dict_name) {
    DoubleBufDict* ddict = _dict_map.at(dict_name);
    return ddict->get_cur_version();
  }

  VirtualDict* get_bg_dict(std::string dict_name) const {
    if (_dict_map.find(dict_name) != _dict_map.end()) {
      DoubleBufDict* ddict = _dict_map.at(dict_name);
      return ddict->get_bg_dict(); 
    } else {
      return nullptr;
    }
  }

  std::string get_bg_version(std::string dict_name) {
    DoubleBufDict* ddict = _dict_map.at(dict_name);
    return ddict->get_bg_version();
  }

  void set_bg_dict(std::string dict_name, VirtualDict* dict) {
    DoubleBufDict* ddict = _dict_map.at(dict_name);
    ddict->set_bg_dict(dict);    
  }

  void release(VirtualDict* dict);

 private:
  Framework() {}
  Framework(const Framework&) {}

 private:
  //VirtualDict* _dict[2]{nullptr, nullptr};
  std::unordered_map<std::string, DoubleBufDict*> _dict_map;
  std::unordered_map<std::string, int> _dict_idx_map;
  std::unordered_map<std::string, bool> _in_mem_map;
  uint32_t _max_val_size;
  uint32_t _dict_split{0};
  std::vector<std::string> _dict_set_path;
  std::atomic_int _status;
};  // class Framework

}  // namespace mcube
}  // namespace rec
