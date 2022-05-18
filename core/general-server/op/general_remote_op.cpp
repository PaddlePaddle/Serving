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

#include "core/general-server/op/general_remote_op.h"
#include <iostream>
#include <sstream>
#include "core/util/include/timer.h"

// paddle inference 2.1 support: FLOAT32, INT64, INT32, UINT8, INT8
// will support: FLOAT16
#define BRPC_MAX_BODY_SIZE 2 * 1024 * 1024 * 1024
const std::string LODABALANCE = "";

namespace baidu {
namespace paddle_serving {
namespace serving {

using baidu::paddle_serving::Timer;
using baidu::paddle_serving::predictor::general_model::Tensor;
using baidu::paddle_serving::predictor::general_model::Request;
using baidu::paddle_serving::predictor::general_model::Response;

brpc::Channel BRPCStub::brpc_channels[MAX_MP_NUM];

brpc::ChannelOptions BRPCStub::options;
std::atomic<int> BRPCStub::inited(0);

int GeneralRemoteOp::inference() {
  LOG(INFO) << "Enter GeneralRemoteOp:inference()";
  int expected = 0;
  std::vector<std::string> op_address = address();
  if (BRPCStub::inited.compare_exchange_strong(expected, 1)) {
    BRPCStub::options.protocol = "baidu_std";
    BRPCStub::options.connection_type = "short";
    BRPCStub::options.timeout_ms = 80000 /*milliseconds*/;
    BRPCStub::options.max_retry = 100;
    brpc::fLU64::FLAGS_max_body_size = BRPC_MAX_BODY_SIZE;

    LOG(ERROR) << "address size: " << op_address.size();
    for (int i = 0; i < op_address.size(); ++i) {
      LOG(INFO) << i + 1 << " address is " << op_address[i].c_str();
      BRPCStub::brpc_channels[i].Init(
          op_address[i].c_str(), LODABALANCE.c_str(), &BRPCStub::options);
    }

    BRPCStub::inited++;
  }
  while (BRPCStub::inited < 2) {
  }

  Timer timeline;
  int64_t start = timeline.TimeStampUS();
  timeline.Start();
  VLOG(2) << "Going to run Remote inference";

  Request* req = (Request*)(get_request_message());
  Response* res = mutable_data<Response>();
  uint64_t log_id = req->log_id();

  brpc::Controller brpc_controllers[MAX_MP_NUM];
  brpc::CallId brpc_callids[MAX_MP_NUM];
  Response brpc_response_tmp;

  size_t i = 0;
  // Init BRPC controllers, callids and stubs
  for (i = 0; i < op_address.size(); ++i) {
    brpc_controllers[i].set_log_id(log_id);
    brpc_callids[i] = brpc_controllers[i].call_id();
  }
  for (i = 0; i < op_address.size(); ++i) {
    baidu::paddle_serving::predictor::general_model::GeneralModelService_Stub
        stub(&BRPCStub::brpc_channels[i]);
    LOG(INFO) << "Sended 1 request to Slave Sever " << i;
    if (0 == i) {
      stub.inference(&brpc_controllers[i], req, res, brpc::DoNothing());
      continue;
    }
    stub.inference(
        &brpc_controllers[i], req, &brpc_response_tmp, brpc::DoNothing());
  }

  LOG(INFO) << "All request are sended, waiting for all responses.";

  // Wait RPC done.
  for (i = 0; i < op_address.size(); ++i) {
    brpc::Join(brpc_callids[i]);
  }

  // Print RPC Results
  for (i = 0; i < op_address.size(); ++i) {
    LOG(INFO) << "brpc_controller_" << i
              << " status:" << brpc_controllers[i].Failed();
    if (!brpc_controllers[i].Failed()) {
      LOG(INFO) << "Received response from "
                << brpc_controllers[i].remote_side()
                << " Latency=" << brpc_controllers[i].latency_us() << "us";
    } else {
      LOG(ERROR) << brpc_controllers[i].ErrorText();
    }
  }
  LOG(INFO) << "All brpc remote stubs joined done.";

  res->set_log_id(log_id);
  res->set_profile_server(req->profile_server());
  int64_t end = timeline.TimeStampUS();
  res->add_profile_time(start);
  res->add_profile_time(end);

  return 0;
}

DEFINE_OP(GeneralRemoteOp);
}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
