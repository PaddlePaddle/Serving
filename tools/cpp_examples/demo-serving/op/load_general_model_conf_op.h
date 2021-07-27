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
#include <memory>

#include "core/predictor/common/inner_common.h"
#include "core/predictor/framework/channel.h"
#include "core/predictor/framework/op_repository.h"
#include "core/predictor/framework/resource.h"
#include "core/predictor/op/op.h"
#include "examples/demo-serving/load_general_model_service.pb.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

class LoadGeneralModelConfOp
    : public OpWithChannel<baidu::paddle_serving::predictor::
                               load_general_model_service::RequestAndResponse> {
 public:
  typedef baidu::paddle_serving::predictor::load_general_model_service::
      RequestAndResponse RequestAndResponse;

  DECLARE_OP(LoadGeneralModelConfOp);

  int inference() {
    LOG(INFO) << "start to call load general model_conf op";
    baidu::paddle_serving::predictor::Resource& resource =
        baidu::paddle_serving::predictor::Resource::instance();

    LOG(INFO) << "get resource pointer done.";
    std::shared_ptr<PaddleGeneralModelConfig> model_config =
        resource.get_general_model_config();

    LOG(INFO) << "get general model config pointer done.";
    resource.print_general_model_config(model_config);

    LOG(INFO) << "print general model config done.";
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
