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

#include "examples/demo-serving/op/int64tensor_echo_op.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

using baidu::paddle_serving::predictor::format::Float32TensorPredictor;
using baidu::paddle_serving::predictor::int64tensor_service::Request;
using baidu::paddle_serving::predictor::int64tensor_service::Response;

int Int64TensorEchoOp::inference() {
  const Request* req = dynamic_cast<const Request*>(get_request_message());
  Response* res = mutable_data<Response>();
  LOG(INFO) << "Receive request in dense service:" << req->ShortDebugString();
  uint32_t sample_size = req->instances_size();
  for (uint32_t si = 0; si < sample_size; si++) {
    Float32TensorPredictor* float32_tensor_res =
        res->mutable_predictions()->Add();
    float32_tensor_res->add_data(1.0);
    float32_tensor_res->add_data(2.0);
    float32_tensor_res->add_shape(2);
    float32_tensor_res->add_shape(1);
  }
  return 0;
}

DEFINE_OP(Int64TensorEchoOp);

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
