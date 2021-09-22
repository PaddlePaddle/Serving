// Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
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

#include "core/general-server/op/general_remote_infer_op.h"
#include <iostream>
#include <memory>
#include <sstream>

#include "core/util/include/timer.h"

namespace baidu {
namespace paddle_serving {
namespace serving {

using baidu::paddle_serving::Timer;
using baidu::paddle_serving::predictor::general_model::Tensor;
using baidu::paddle_serving::predictor::general_model::Response;
using baidu::paddle_serving::predictor::general_model::Request;

#define BRPC_MAX_BODY_SIZE 512 * 1024 * 1024
const std::string IP_PORT = "127.0.0.1:9393";

int GeneralRemoteInferOp::connect(const std::string server_port) {
  brpc::fLU64::FLAGS_max_body_size = BRPC_MAX_BODY_SIZE;
  if (_api.create(gen_desc(server_port)) != 0) {
    LOG(ERROR) << "Predictor Creation Failed";
    return -1;
  }
  // _api.thrd_initialize();
  _api.thrd_initialize();
  std::string variant_tag;
  // predictor is bound to request with brpc::Controller
  _predictor = _api.fetch_predictor("general_model", &variant_tag);
  if (_predictor == NULL) {
    LOG(ERROR) << "Failed fetch predictor so predict error!";
    return -1;
  }
  // predict_res_batch.set_variant_tag(variant_tag);
  VLOG(2) << "fetch general model predictor done.";
  VLOG(2) << "variant_tag:" << variant_tag;
  VLOG(2) << "max body size : " << brpc::fLU64::FLAGS_max_body_size;
  inited = true;
  return 0;
}

std::shared_ptr<SDKConf> GeneralRemoteInferOp::gen_desc(
    const std::string server_port) {
  // default config for brpc
  std::shared_ptr<SDKConf> sdk_conf_sharedptr = std::make_shared<SDKConf>();

  Predictor* predictor = sdk_conf_sharedptr->add_predictors();
  predictor->set_name("general_model");
  predictor->set_service_name(
      "baidu.paddle_serving.predictor.general_model.GeneralModelService");
  predictor->set_endpoint_router("WeightedRandomRender");
  predictor->mutable_weighted_random_render_conf()->set_variant_weight_list(
      "100");
  VariantConf* predictor_var = predictor->add_variants();
  predictor_var->set_tag("default_tag_1");
  std::string cluster = "list://" + server_port;
  predictor_var->mutable_naming_conf()->set_cluster(cluster);

  VariantConf* var = sdk_conf_sharedptr->mutable_default_variant_conf();
  var->set_tag("default");
  var->mutable_connection_conf()->set_connect_timeout_ms(2000);
  var->mutable_connection_conf()->set_rpc_timeout_ms(2000000);
  var->mutable_connection_conf()->set_connect_retry_count(2);
  var->mutable_connection_conf()->set_max_connection_per_host(100);
  var->mutable_connection_conf()->set_hedge_request_timeout_ms(-1);
  var->mutable_connection_conf()->set_hedge_fetch_retry_count(2);
  var->mutable_connection_conf()->set_connection_type("pooled");

  var->mutable_naming_conf()->set_cluster_filter_strategy("Default");
  var->mutable_naming_conf()->set_load_balance_strategy("la");

  var->mutable_rpc_parameter()->set_compress_type(0);
  var->mutable_rpc_parameter()->set_package_size(20);
  var->mutable_rpc_parameter()->set_protocol("baidu_std");
  var->mutable_rpc_parameter()->set_max_channel_per_request(3);

  return sdk_conf_sharedptr;
}

int GeneralRemoteInferOp::inference() {
  Timer timeline;
  int64_t start = timeline.TimeStampUS();
  timeline.Start();
  VLOG(2) << "Going to run inference";
  if (!inited) {
    connect(IP_PORT);
  }
  const std::vector<std::string> pre_node_names = pre_names();
  if (pre_node_names.size() > 1) {
    LOG(ERROR) << "This op(" << op_name()
               << ") can only have one predecessor op, but received "
               << pre_node_names.size();
    return -1;
  }
  Request* req = nullptr;
  std::string pre_name;
  if (pre_node_names.size() == 0) {
    req = (Request*)(get_request_message());
  } else {
    pre_name = pre_node_names[0];
    req = (Request*)get_depend_argument<Request>(pre_name);
  }

  // const Request *input_blob = get_depend_argument<Request>(pre_name);
  if (!req) {
    LOG(ERROR) << "req is nullptr,error";
    return -1;
  }
  uint64_t log_id = req->log_id();
  VLOG(2) << "(logid=" << log_id << ") Get precedent op name: " << pre_name;

  Response* res = mutable_data<Response>();
  if (!res) {
    LOG(ERROR) << "res is nullptr,error";
    return -1;
  }
  res->Clear();

  if (!req) {
    LOG(ERROR) << "(logid=" << log_id
               << ") Failed mutable depended argument, op:" << pre_name;
    return -1;
  }

  if (_predictor->inference(req, res) != 0) {
    LOG(ERROR) << "failed call predictor with req: " << req->ShortDebugString();
    return -1;
  }

  res->set_log_id(log_id);
  res->set_profile_server(req->profile_server());
  int64_t end = timeline.TimeStampUS();
  res->add_profile_time(start);
  res->add_profile_time(end);
  return 0;
}

GeneralRemoteInferOp::~GeneralRemoteInferOp() { _api.thrd_clear(); }
DEFINE_OP(GeneralRemoteInferOp);

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
