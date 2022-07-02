// Copyright (c) 2021 PaddlePaddle Authors. All Rights Reserved.
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

#include "core/general-client/include/brpc_client.h"
#include "core/sdk-cpp/include/common.h"
#include "core/util/include/timer.h"
#include "core/sdk-cpp/builtin_format.pb.h"
#include "core/sdk-cpp/general_model_service.pb.h"
DEFINE_bool(profile_client, false, "");
DEFINE_bool(profile_server, false, "");
#define BRPC_MAX_BODY_SIZE 512 * 1024 * 1024

namespace baidu {
namespace paddle_serving {
namespace client {

using baidu::paddle_serving::Timer;
using baidu::paddle_serving::predictor::general_model::Request;
using baidu::paddle_serving::predictor::general_model::Response;
using baidu::paddle_serving::predictor::general_model::Tensor;

using configure::SDKConf;
using configure::VariantConf;
using configure::Predictor;
using configure::VariantConf;

int ServingBrpcClient::connect(const std::string server_port) {
  brpc::fLU64::FLAGS_max_body_size = BRPC_MAX_BODY_SIZE;
  if (_api.create(gen_desc(server_port)) != 0) {
    LOG(ERROR) << "Predictor Creation Failed";
    return -1;
  }
  // _api.thrd_initialize();
  return 0;
}

std::string ServingBrpcClient::gen_desc(const std::string server_port) {
  // default config for brpc
  SDKConf sdk_conf;

  Predictor* predictor = sdk_conf.add_predictors();
  predictor->set_name("general_model");
  predictor->set_service_name("baidu.paddle_serving.predictor.general_model.GeneralModelService");
  predictor->set_endpoint_router("WeightedRandomRender");
  predictor->mutable_weighted_random_render_conf()->set_variant_weight_list("100");
  VariantConf* predictor_var = predictor->add_variants();
  predictor_var->set_tag("default_tag_1");
  std::string cluster = "list://" + server_port;
  predictor_var->mutable_naming_conf()->set_cluster(cluster);

  VariantConf* var = sdk_conf.mutable_default_variant_conf();
  var->set_tag("default");
  var->mutable_connection_conf()->set_connect_timeout_ms(2000);
  var->mutable_connection_conf()->set_rpc_timeout_ms(200000);
  var->mutable_connection_conf()->set_connect_retry_count(2);
  var->mutable_connection_conf()->set_max_connection_per_host(100);
  var->mutable_connection_conf()->set_hedge_request_timeout_ms(-1);
  var->mutable_connection_conf()->set_hedge_fetch_retry_count(2);
  var->mutable_connection_conf()->set_connection_type("pooled");
  var->mutable_connection_conf()->set_connect_timeout_ms(2000);

  var->mutable_naming_conf()->set_cluster_filter_strategy("Default");
  var->mutable_naming_conf()->set_load_balance_strategy("la");

  var->mutable_rpc_parameter()->set_compress_type(0);
  var->mutable_rpc_parameter()->set_package_size(20);
  var->mutable_rpc_parameter()->set_protocol("baidu_std");
  var->mutable_rpc_parameter()->set_max_channel_per_request(3);

  return sdk_conf.SerializePartialAsString();
}

int ServingBrpcClient::predict(const PredictorInputs& inputs,
                               PredictorOutputs& outputs,
                               const std::vector<std::string>& fetch_name,
                               const uint64_t log_id) {
  Timer timeline;
  int64_t preprocess_start = timeline.TimeStampUS();
  // thread initialize for StubTLS
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
  Request req;
  req.set_log_id(log_id);
  for (auto &name : fetch_name) {
    req.add_fetch_var_names(name);
  }

  if (PredictorInputs::GenProto(inputs, _feed_name_to_idx, _feed_name, req) != 0) {
    LOG(ERROR) << "Failed to preprocess req!";
    return -1;
  }

  int64_t preprocess_end = timeline.TimeStampUS();
  int64_t client_infer_start = timeline.TimeStampUS();
  Response res;

  int64_t client_infer_end = 0;
  int64_t postprocess_start = 0;
  int64_t postprocess_end = 0;

  if (FLAGS_profile_server) {
    req.set_profile_server(true);
  }

  res.Clear();
  if (_predictor->inference(&req, &res) != 0) {
    LOG(ERROR) << "failed call predictor with req: " << req.ShortDebugString();
    return -1;
  }

  client_infer_end = timeline.TimeStampUS();
  postprocess_start = client_infer_end;
  if (PredictorOutputs::ParseProto(res, fetch_name, _fetch_name_to_type, outputs) != 0) {
    LOG(ERROR) << "Failed to post_process res!";
    return -1;
  }
  postprocess_end = timeline.TimeStampUS();

  if (FLAGS_profile_client) {
    std::ostringstream oss;
    oss << "PROFILE\t"
        << "pid:" << getpid() << "\t"
        << "prepro_0:" << preprocess_start << " "
        << "prepro_1:" << preprocess_end << " "
        << "client_infer_0:" << client_infer_start << " "
        << "client_infer_1:" << client_infer_end << " ";
    if (FLAGS_profile_server) {
      int op_num = res.profile_time_size() / 2;
      for (int i = 0; i < op_num; ++i) {
        oss << "op" << i << "_0:" << res.profile_time(i * 2) << " ";
        oss << "op" << i << "_1:" << res.profile_time(i * 2 + 1) << " ";
      }
    }

    oss << "postpro_0:" << postprocess_start << " ";
    oss << "postpro_1:" << postprocess_end;

    fprintf(stderr, "%s\n", oss.str().c_str());
  }

  // release predictor
  _api.thrd_clear();

  std::ostringstream oss;
  oss << "[client]"
      << "logid=" << log_id <<",";
  if (FLAGS_profile_client) {
    double pre_cost = (preprocess_end - preprocess_start) / 1000.0;
    double infer_cost = (client_infer_end - client_infer_start) / 1000.0;
    double post_cost = (postprocess_end - postprocess_start) / 1000.0;
    oss << "client_pre_cost=" << pre_cost << "ms,"
        << "client_infer_cost=" << infer_cost << "ms,"
        << "client_post_cost=" << post_cost << "ms,";
  }
  double client_cost = (postprocess_end - preprocess_start) / 1000.0;
  oss << "client_cost=" << client_cost << "ms,";

  int op_num = res.profile_time_size() / 2;
  if (FLAGS_profile_server) {
    for (int i = 0; i < op_num - 1; ++i) {
      double t = (res.profile_time(i * 2 + 1)
                 - res.profile_time(i * 2)) / 1000.0;
      oss << "op" << i << "=" << t << "ms,";
    }
  }
  if (op_num > 0) {
    int i = op_num - 1;
    double server_cost = (res.profile_time(i * 2 + 1)
                 - res.profile_time(i * 2)) / 1000.0;
    oss << "server_cost=" << server_cost << "ms.";
  }
  LOG(INFO) << oss.str();

  return 0;
}

}  // namespace general_model
}  // namespace paddle_serving
}  // namespace baidu
