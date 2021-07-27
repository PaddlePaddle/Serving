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

#include "examples/demo-serving/op/sparse_echo_op.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

int SparseEchoOp::inference() {
  // Every op can obtain request message by:
  // get_request_message()
  const Request* req = dynamic_cast<const Request*>(get_request_message());

  // Each op can obtain self-writable-data by:
  // mutable_data()
  Response* res = mutable_data<Response>();

  // You can get the channel/data of depended ops by:
  // get/mutable_depend_argment()
  // ...

  LOG(INFO) << "Receive request in sparse service:" << req->ShortDebugString();
  uint32_t sample_size = req->instances_size();
  for (uint32_t si = 0; si < sample_size; si++) {
    SparsePrediction* sparse_res = res->mutable_predictions()->Add();
    sparse_res->add_categories(100.0 + si * 0.1);
    sparse_res->add_categories(200.0 + si * 0.1);
  }
  return 0;
}

DEFINE_OP(SparseEchoOp);

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
