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

#include "core/general-server/op/general_single_out_op.h"
#include <iostream>
#include <sstream>
#include "core/util/include/timer.h"

namespace baidu {
namespace paddle_serving {
namespace serving {

using baidu::paddle_serving::Timer;
using baidu::paddle_serving::predictor::general_model::Tensor;
using baidu::paddle_serving::predictor::general_model::Request;
using baidu::paddle_serving::predictor::general_model::Response;
// paddle inference 2.1 support: FLOAT32, INT64, INT32, UINT8, INT8
// will support: FLOAT16

int GeneralSingleOutOp::inference() {
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

  // package request
  // get the response from anyobe of preNodes
  std::string pre_name = pre_node_names[0];
  Response* res_pre = (Response*)get_depend_argument<Response>(pre_name);
  uint64_t log_id = res_pre->log_id();
  Response* res_out = mutable_data<Response>();
  if (!res_out) {
    LOG(ERROR) << "(logid=" << log_id << ") Failed get GeneralBlob";
    return -1;
  }
  res_out->Clear();
  res_out->Swap(res_pre);

  timeline.Pause();
  int64_t end = timeline.TimeStampUS();
  VLOG(2) << "(logid=" << log_id << ") read data from client success";
  if(print_count < 10){
    std::cout << "MyOpName = " << op_name() <<  " and Response = " << res_out->DebugString() 
    <<  " and timecost = " << end -start << std::endl;
    print_count++;
  }
  
  return 0;
}

DEFINE_OP(GeneralSingleOutOp);
}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu

