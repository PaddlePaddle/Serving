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

#include "core/cube/cube-server/cube.pb.h"
#include "core/cube/cube-server/include/cube/rw_lock.h"
#include "core/cube/cube-server/include/cube/virtual_dict.h"

namespace rec {
namespace mcube {

struct Status {
  enum StatusCode { F_RUNNING = 0, F_LOADING };
};

class Framework {
 public:
  static Framework* instance();

 public:
  ~Framework();

  int init(std::string dict_name, uint32_t dict_split, bool in_mem);

  int destroy();

  int status(std::string dict_name, BUTIL_RAPIDJSON_NAMESPACE::Document* res);

  int seek(const DictRequest* req, DictResponse* res);

  int reload(std::string dict_name, const std::string& v_path);

  int patch(std::string dict_name, const std::string& v_path);

  // load dict base
  int bg_load_base(std::string dict_name, const std::string& v_path);

  // load dict patch
  int bg_load_patch(std::string dict_name, const std::string& v_path);

  int bg_unload();

  int bg_switch();

  int enable(std::string dict_name, const std::string& version);

 private:
  void init_dict(std::string dict_name, uint32_t dict_split);

  VirtualDict* create_dict();

 private:
  VirtualDict* get_cur_dict(std::string dict_name) {
    _rw_lock.r_lock();
    int _dict_idx = _dict_idx_map[dict_name];
    std::vector<VirtualDict*> _dict = _dict_map[dict_name];
    VirtualDict* dict = _dict[_dict_idx];
    dict->atom_inc_seek_num();
    _rw_lock.unlock();
    return dict;
  }

  std::string get_cur_version(std::string dict_name) {
    _rw_lock.r_lock();
    int _dict_idx = _dict_idx_map[dict_name];
    std::vector<VirtualDict*> _dict = _dict_map[dict_name];
    VirtualDict* dict = _dict[_dict_idx];
    std::string version = dict->version();
    _rw_lock.unlock();
    return version;
  }

  VirtualDict* get_bg_dict(std::string dict_name) const { 
    int _dict_idx = _dict_idx_map.at(dict_name);
    std::vector<VirtualDict*> _dict = _dict_map.at(dict_name);
    return _dict[1 - _dict_idx]; 
  }

  std::string get_bg_version(std::string dict_name) {
    _bg_rw_lock.r_lock();
    int _dict_idx = _dict_idx_map.at(dict_name);
    std::vector<VirtualDict*> _dict = _dict_map[dict_name];
    VirtualDict* dict = _dict[1 - _dict_idx];
    std::string version = "";
    if (dict) {
      version = dict->guard_version();
    }
    _bg_rw_lock.unlock();
    return version;
  }

  void set_bg_dict(std::string dict_name, VirtualDict* dict) {
    _bg_rw_lock.w_lock();
    int _dict_idx = _dict_idx_map[dict_name];
    std::vector<VirtualDict*>& _dict = _dict_map[dict_name];
    _dict[1 - _dict_idx] = dict;
    _bg_rw_lock.unlock();
  }

  void release(VirtualDict* dict);

 private:
  Framework() {}
  Framework(const Framework&) {}

 private:
  //VirtualDict* _dict[2]{nullptr, nullptr};
  std::unordered_map<std::string, std::vector<VirtualDict*>> _dict_map;
  std::unordered_map<std::string, int> _dict_idx_map;
  std::unordered_map<std::string, bool> _in_mem_map;
  std::unordered_map<std::string, uint32_t> _max_val_size_map;
  std::unordered_map<std::string, std::string> _dict_path_map;
  //int _dict_idx{0};
  //std::string _dict_path{""};
  //bool _in_mem{true};
  //std::atomic_int _status;
  RWLock _rw_lock;
  RWLock _bg_rw_lock;
  //uint32_t _max_val_size{0};
  uint32_t _dict_split{0};
  std::vector<std::string> _dict_set_path;
};  // class Framework

}  // namespace mcube
}  // namespace rec
