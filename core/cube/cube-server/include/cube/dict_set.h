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

#include "core/cube/cube-server/include/cube/dict.h"
#include "core/cube/cube-server/include/cube/virtual_dict.h"

namespace rec {
namespace mcube {

class DictSet : public VirtualDict {
 public:
  explicit DictSet(int dict_split);

  virtual ~DictSet();

  virtual int load(const std::vector<std::string>& dict_path,
                   bool in_mem,
                   const std::string& v_path);

  virtual int destroy();

  virtual const std::string& version();

  virtual std::string guard_version();

  virtual void set_base_dict(const VirtualDict* dict);

  virtual bool seek(uint64_t key, char* buff, uint64_t* buff_size);

  virtual void atom_inc_seek_num();

  virtual void atom_dec_seek_num();

  virtual uint32_t atom_seek_num();

 private:
  std::atomic<uint32_t> _seek_num{0};
  std::vector<DictPtr> _dict_set;
  int _dict_split{0};
  std::string _version{""};
  RWLock _rw_lock;
};  // DictSet

}  // namespace mcube
}  // namespace rec
