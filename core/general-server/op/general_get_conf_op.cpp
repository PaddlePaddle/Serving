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

#include "core/general-server/op/general_get_conf_op.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>

namespace baidu {
namespace paddle_serving {
namespace serving {

using baidu::paddle_serving::predictor::general_model::Request;
using baidu::paddle_serving::predictor::general_model::Response;

int GeneralGetConfOp::inference() {
  // reade request from client
  const Request *req = dynamic_cast<const Request *>(get_request_message());

  baidu::paddle_serving::predictor::Resource &resource =
      baidu::paddle_serving::predictor::Resource::instance();

  std::string conf_str = resource.get_general_model_conf_str();
  Response *res = mutable_data<Response>();
  res->set_config_str(conf_str);
  return 0;
}
DEFINE_OP(GeneralGetConfOp);
}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
