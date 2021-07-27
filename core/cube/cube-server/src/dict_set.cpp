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

#include "core/cube/cube-server/include/cube/dict_set.h"

namespace rec {
namespace mcube {

DictSet::DictSet(int dict_split) : VirtualDict(), _dict_split(dict_split) {
  _dict_set.resize(_dict_split);
}

DictSet::~DictSet() {}

void DictSet::set_base_dict(const VirtualDict* dict) {
  const DictSet* dict_set = static_cast<const DictSet*>(dict);
  for (size_t i = 0; i < _dict_set.size(); ++i) {
    if (!_dict_set[i]) {
      _dict_set[i] = std::make_shared<Dict>();
    }
    _dict_set[i]->set_base_dict(dict_set->_dict_set[i].get());
  }
}

int DictSet::load(const std::vector<std::string>& dict_path,
                  bool in_mem,
                  const std::string& v_path) {
  if ((uint32_t)_dict_split != dict_path.size()) {
    return E_DATA_ERROR;
  }

  for (size_t i = 0; i < dict_path.size(); ++i) {
    if (!_dict_set[i]) {
      _dict_set[i] = std::make_shared<Dict>();
    }

    if (_dict_set[i]->load(dict_path[i], in_mem, v_path) != E_OK) {
      LOG(ERROR) << "dict split[" << i << "] load failed";
      return E_DATA_ERROR;
    }
  }

  _rw_lock.w_lock();
  _version = (v_path == "") ? "" : v_path.substr(1);
  _rw_lock.unlock();

  return E_OK;
}

int DictSet::destroy() {
  for (size_t i = 0; i < _dict_set.size(); ++i) {
    if (_dict_set[i]->destroy() != E_OK) {
      LOG(WARNING) << "dict split[" << i << "] destory failed";
    }
  }

  return E_OK;
}

const std::string& DictSet::version() { return _version; }

std::string DictSet::guard_version() {
  _rw_lock.r_lock();
  std::string version = _version;
  _rw_lock.unlock();
  return version;
}

bool DictSet::seek(uint64_t key, char* buff, uint64_t* buff_size) {
  return _dict_set[key % _dict_split]->seek(key, buff, buff_size);
}

void DictSet::atom_inc_seek_num() { ++_seek_num; }

void DictSet::atom_dec_seek_num() { --_seek_num; }

uint32_t DictSet::atom_seek_num() { return _seek_num; }

}  // namespace mcube
}  // namespace rec
