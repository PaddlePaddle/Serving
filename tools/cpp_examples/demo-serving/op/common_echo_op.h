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
#include "examples/demo-serving/echo_service.pb.h"

#include "core/predictor/common/inner_common.h"
#include "core/predictor/framework/channel.h"
#include "core/predictor/framework/op_repository.h"
#include "core/predictor/op/op.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

class CommonEchoOp
    : public OpWithChannel<
          baidu::paddle_serving::predictor::echo_service::RequestAndResponse> {
 public:
  typedef baidu::paddle_serving::predictor::echo_service::RequestAndResponse
      RequestAndResponse;

  DECLARE_OP(CommonEchoOp);

  int inference() {
    const RequestAndResponse* req =
        dynamic_cast<const RequestAndResponse*>(get_request_message());

    RequestAndResponse* data = mutable_data<RequestAndResponse>();

    data->CopyFrom(*req);

    return 0;
  }
};

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
