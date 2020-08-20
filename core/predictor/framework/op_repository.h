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
#include "core/predictor/common/inner_common.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

#define REGISTER_OP(op)                                                       \
  ::baidu::paddle_serving::predictor::OpRepository::instance().regist_op<op>( \
      #op)

class Op;

class Factory {
 public:
  virtual Op* get_op() = 0;
  virtual void return_op(Op* op) = 0;
};

template <typename OP_TYPE>
class OpFactory : public Factory {
 public:
  Op* get_op() { return butil::get_object<OP_TYPE>(); }

  void return_op(Op* op) {
    butil::return_object<OP_TYPE>(dynamic_cast<OP_TYPE*>(op));
  }

  static OpFactory<OP_TYPE>& instance() {
    static OpFactory<OP_TYPE> ins;
    return ins;
  }
};

class OpRepository {
 public:
  typedef boost::unordered_map<std::string, Factory*> ManagerMap;

  OpRepository() {}
  ~OpRepository() {}

  static OpRepository& instance() {
    static OpRepository repo;
    return repo;
  }

  template <typename OP_TYPE>
  void regist_op(std::string op_type) {
    _repository[op_type] = &OpFactory<OP_TYPE>::instance();
    char err_str[ERROR_STRING_LEN];
    snprintf(
        err_str, ERROR_STRING_LEN - 1, "Succ regist op: %s", op_type.c_str());
    RAW_LOG(INFO, err_str);
  }

  Op* get_op(std::string op_type);

  void return_op(Op* op);

  void return_op(const std::string& op_type, Op* op);

 private:
  ManagerMap _repository;
};

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
