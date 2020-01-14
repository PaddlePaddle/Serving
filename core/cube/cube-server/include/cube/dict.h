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

#include <stdint.h>

#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include "core/cube/cube-server/include/cube/rw_lock.h"
#include "core/cube/cube-server/include/cube/slim_hash_map.h"
#include "core/cube/cube-server/include/cube/virtual_dict.h"

namespace rec {
namespace mcube {

class Dict : public VirtualDict {
 public:
  Dict();

  virtual ~Dict();

  /*
  void set_base_dict(const Dict * dict){
      _base_dict = dict;
  }
  */

  virtual void set_base_dict(const VirtualDict* dict) {
    _base_dict = static_cast<const Dict*>(dict);
  }

  int load(const std::string& dict_path,
           bool in_mem,
           const std::string& v_path);

  int destroy();

  bool seek(uint64_t key, char* buff, uint64_t* buff_size);

  const std::string& version();  // no lock, used by framework seek

  std::string guard_version();

  void atom_inc_seek_num();

  void atom_dec_seek_num();

  uint32_t atom_seek_num();

 private:
  int load_index(const std::string& dict_path, const std::string& v_path);

  int load_data(const std::string& dict_path, const std::string& v_path);

  int load_data_mmap(const std::string& dict_path, const std::string& v_path);

  void set_version(const std::string& v_path);

 private:
  struct DataBlock {
    DataBlock() : size(0), fd(-1) {}

    std::shared_ptr<char> s_data;
    uint32_t size;
    int fd;
  };

 private:
  // boost::unordered_map<uint64_t, uint64_t> _table;
  slim_hash_map<uint64_t, uint64_t> _slim_table;
  std::vector<DataBlock> _block_set;
  std::atomic<uint32_t> _seek_num;
  const Dict* _base_dict;
  std::string _version;
  RWLock _rw_lock;
};  // class Dict

typedef std::shared_ptr<Dict> DictPtr;

}  // namespace mcube
}  // namespace rec
