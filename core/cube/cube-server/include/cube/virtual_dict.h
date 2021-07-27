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

#include <string>
#include <vector>

#include "core/cube/cube-server/include/cube/error.h"

namespace rec {
namespace mcube {

class VirtualDict {
 public:
  VirtualDict() {}

  virtual ~VirtualDict() {}

  virtual int load(const std::string& /*dict_path*/,
                   bool /*in_mem*/,
                   const std::string& /*v_path*/) {
    return E_NOT_IMPL;
  }

  virtual int load(const std::vector<std::string>& /*dict_path*/,
                   bool /*in_mem*/,
                   const std::string& /*v_path*/) {
    return E_NOT_IMPL;
  }

  virtual int destroy() { return E_NOT_IMPL; }

  virtual const std::string& version() {
    static std::string UNKNOWN_VERSION = "UNKNOWN";
    return UNKNOWN_VERSION;
  }

  virtual std::string guard_version() {
    static std::string UNKNOWN_VERSION = "UNKNOWN";
    return UNKNOWN_VERSION;
  }

  virtual void set_base_dict(const VirtualDict* dict) = 0;

  virtual bool seek(uint64_t key, char* buff, uint64_t* buff_size) = 0;

  virtual void atom_inc_seek_num() = 0;

  virtual void atom_dec_seek_num() = 0;
  virtual uint32_t atom_seek_num() = 0;
};  // class VirtualDict

}  // namespace mcube
}  // namespace rec
