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

#include "core/predictor/framework/op_repository.h"
#include <string>
#include "core/predictor/op/op.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

Op* OpRepository::get_op(std::string op_type) {
  ManagerMap::iterator iter = _repository.find(op_type);
  Op* op = NULL;
  if (iter != _repository.end()) {
    op = (iter->second)->get_op();
  } else {
    LOG(ERROR) << "Try to create unknown op[" << op_type << "]";
  }
  return op;
}

void OpRepository::return_op(Op* op) {
  if (op == NULL) {
    LOG(ERROR) << "Try to return NULL op";
    return;
  }
  ManagerMap::iterator iter = _repository.find(op->type());
  if (iter != _repository.end()) {
    iter->second->return_op(op);
  } else {
    LOG(ERROR) << "Try to return unknown op[" << op << "], op_type["
               << op->type() << "].";
  }
}

void OpRepository::return_op(const std::string& op_type, Op* op) {
  if (op == NULL) {
    LOG(ERROR) << "Try to return NULL op";
    return;
  }
  ManagerMap::iterator iter = _repository.find(op_type);
  if (iter != _repository.end()) {
    iter->second->return_op(op);
  } else {
    LOG(ERROR) << "Try to return unknown op[" << op << "], op_type[" << op_type
               << "].";
  }
}

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
