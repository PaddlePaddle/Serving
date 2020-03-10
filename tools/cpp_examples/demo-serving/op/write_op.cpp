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

#include <google/protobuf/text_format.h>
#include <string>

#ifdef BCLOUD
#include "pb_to_json.h"  // NOLINT
#else
#include "json2pb/pb_to_json.h"
#endif

#include "core/predictor/framework/memory.h"
#include "examples/demo-serving/op/write_op.h"

#ifndef BCLOUD
using json2pb::ProtoMessageToJson;
#endif

namespace baidu {
namespace paddle_serving {
namespace serving {

using baidu::paddle_serving::predictor::format::XImageResInstance;
using baidu::paddle_serving::predictor::image_classification::ClassifyResponse;
using baidu::paddle_serving::predictor::image_classification::Response;

int WriteOp::inference() {
  const ClassifyResponse* classify_out =
      get_depend_argument<ClassifyResponse>("image_classify_op");
  if (!classify_out) {
    LOG(ERROR) << "Failed mutable depended argument, op:"
               << "image_classify_op";
    return -1;
  }

  Response* res = mutable_data<Response>();
  if (!res) {
    LOG(ERROR) << "Failed mutable output response in op:"
               << "WriteOp";
    return -1;
  }

  // transfer classify output message into json format
  std::string err_string;
  uint32_t sample_size = classify_out->predictions_size();
  for (uint32_t si = 0; si < sample_size; si++) {
    XImageResInstance* ins = res->add_predictions();
    if (!ins) {
      LOG(ERROR) << "Failed add one prediction ins";
      return -1;
    }
    std::string* text = ins->mutable_response_json();
    if (!ProtoMessageToJson(classify_out->predictions(si), text, &err_string)) {
      LOG(ERROR) << "Failed convert message["
                 << classify_out->predictions(si).ShortDebugString()
                 << "], err: " << err_string;
      return -1;
    }
  }

  LOG(INFO) << "Succ write json:" << classify_out->ShortDebugString();

  return 0;
}

DEFINE_OP(WriteOp);

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
