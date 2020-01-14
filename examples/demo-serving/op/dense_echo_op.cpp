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

#include "examples/demo-serving/op/dense_echo_op.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

using baidu::paddle_serving::predictor::format::DensePrediction;
using baidu::paddle_serving::predictor::dense_service::Request;
using baidu::paddle_serving::predictor::dense_service::Response;

int DenseEchoOp::inference() {
  const Request* req = dynamic_cast<const Request*>(get_request_message());
  Response* res = mutable_data<Response>();
  LOG(INFO) << "Receive request in dense service:" << req->ShortDebugString();
  uint32_t sample_size = req->instances_size();
  for (uint32_t si = 0; si < sample_size; si++) {
    DensePrediction* dense_res = res->mutable_predictions()->Add();
    dense_res->add_categories(100.0 + si * 0.1);
    dense_res->add_categories(200.0 + si * 0.1);
  }
  return 0;
}

DEFINE_OP(DenseEchoOp);

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
