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

#include "core/general-server/op/general_middle_op.h"
#include <iostream>
#include <sstream>
#include "core/util/include/timer.h"

namespace baidu {
namespace paddle_serving {
namespace serving {

const std::string NEED_FEEDVAR_NAME = "input_mask";
using baidu::paddle_serving::Timer;
using baidu::paddle_serving::predictor::general_model::Tensor;
using baidu::paddle_serving::predictor::general_model::Request;
using baidu::paddle_serving::predictor::general_model::Response;
// paddle inference 2.1 support: FLOAT32, INT64, INT32, UINT8, INT8
// will support: FLOAT16

int GeneralMiddleOp::inference() {
  VLOG(2) << "Going to run inference";
  Timer timeline;
  int64_t start = timeline.TimeStampUS();
  timeline.Start();
  const std::vector<std::string> pre_node_names = pre_names();
  if (pre_node_names.size() < 1) {
    LOG(ERROR) << "This op(" << op_name()
               << ") at least have one predecessor op, but received "
               << pre_node_names.size();
    return -1;
  }
  VLOG(2) << "pre node names size: " << pre_node_names.size();

  // read request from client
  const Request *original_req =
      dynamic_cast<const Request *>(get_request_message());
  if (!original_req) {
    LOG(ERROR) << "Failed get request message";
    return -1;
  }
  uint64_t log_id = original_req->log_id();

  Request *req_out = mutable_data<Request>();
  if (!req_out) {
    LOG(ERROR) << "(logid=" << log_id << ") Failed get GeneralBlob";
    return -1;
  }
  req_out->Clear();
  req_out->set_log_id(log_id);
  req_out->set_profile_server(original_req->profile_server());

  // var_num means the number of feed_var.
  int var_num = original_req->tensor_size();
  VLOG(2) << "(logid=" << log_id << ") var num: " << var_num
          << ") start to call load general model_conf op";

  // package request
  // get the 'input_mask' from original_req
  for (int i = 0; i < var_num; ++i) {
    const Tensor &tensor = original_req->tensor(i);
    if (tensor.name() != NEED_FEEDVAR_NAME) {
      continue;
    }
    *req_out->add_tensor() = tensor;
    break;
  }

  // package request
  // get the response from anyobe of preNodes
  std::string pre_name = pre_node_names[0];
  Response *res = (Response *)get_depend_argument<Response>(pre_name);
  for (int i = 0; i < res->outputs_size(); ++i) {
    for (int j = 0; j < res->outputs(i).tensor_size(); ++j) {
      *req_out->add_tensor() = res->outputs(i).tensor(j);
    }
  }

  timeline.Pause();
  int64_t end = timeline.TimeStampUS();
  VLOG(2) << "(logid=" << log_id << ") read data from client success";
  if(print_count <10){
    std::cout << "MyOpName = " << op_name() << "and original_req = " << original_req->DebugString() << 
    " and Response = " << res->DebugString() << "and ReqOut = " << req_out->DebugString() 
    << " and timecost = " << end -start << std::endl;
    print_count++;
  }
  
  return 0;
}

DEFINE_OP(GeneralMiddleOp);
}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu

