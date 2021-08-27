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
enum ProtoDataType { P_INT64, P_FLOAT32, P_INT32, P_STRING };

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

static int pre_process(const PredictorInputs& inputs,
                        const std::map<std::string, int>& feed_name_to_idx,
                        const std::vector<std::string>& feed_name,
                        Request& req) {
  const std::map<std::string, std::vector<float>>& float_feed_map = inputs.float_data_map();
  const std::map<std::string, std::vector<int64_t>>& int64_feed_map = inputs.int64_data_map();
  const std::map<std::string, std::vector<int32_t>>& int32_feed_map = inputs.int_data_map();
  const std::map<std::string, std::string>& string_feed_map = inputs.string_data_map();
  const std::map<std::string, std::vector<int>>& shape_map = inputs.shape_map();
  const std::map<std::string, std::vector<int>>& lod_map = inputs.lod_map();

  VLOG(2) << "float feed name size: " << float_feed_map.size();
  VLOG(2) << "int feed name size: " << int64_feed_map.size();
  VLOG(2) << "string feed name size: " << string_feed_map.size();

  // batch is already in Tensor.

  for (std::map<std::string, std::vector<float>>::const_iterator iter = float_feed_map.begin();
        iter != float_feed_map.end();
        ++iter) {
    std::string name = iter->first;
    const std::vector<float>& float_data = iter->second;
    const std::vector<int>& float_shape = shape_map.at(name);
    const std::vector<int>& float_lod = lod_map.at(name);
    std::map<std::string, int>::const_iterator feed_name_it = feed_name_to_idx.find(name);
    if (feed_name_it == feed_name_to_idx.end()) {
      LOG(ERROR) << "Do not find [" << name << "] in feed_map!";
      return -1;
    }
    int idx = feed_name_to_idx.at(name);
    VLOG(2) << "prepare float feed " << name << " idx " << idx;
    int total_number = float_data.size();
    Tensor *tensor = req.add_tensor();

    VLOG(2) << "prepare float feed " << name << " shape size "
            << float_shape.size();
    for (uint32_t j = 0; j < float_shape.size(); ++j) {
      tensor->add_shape(float_shape[j]);
    }
    for (uint32_t j = 0; j < float_lod.size(); ++j) {
      tensor->add_lod(float_lod[j]);
    }
    tensor->set_elem_type(P_FLOAT32);

    tensor->set_name(feed_name[idx]);
    tensor->set_alias_name(name);

    tensor->mutable_float_data()->Resize(total_number, 0);
    memcpy(tensor->mutable_float_data()->mutable_data(), float_data.data(), total_number * sizeof(float));
  }

  for (std::map<std::string, std::vector<int64_t>>::const_iterator iter = int64_feed_map.begin();
        iter != int64_feed_map.end();
        ++iter) {
    std::string name = iter->first;
    const std::vector<int64_t>& int64_data = iter->second;
    const std::vector<int>& int64_shape = shape_map.at(name);
    const std::vector<int>& int64_lod = lod_map.at(name);
    std::map<std::string, int>::const_iterator feed_name_it = feed_name_to_idx.find(name);
    if (feed_name_it == feed_name_to_idx.end()) {
      LOG(ERROR) << "Do not find [" << name << "] in feed_map!";
      return -1;
    }
    int idx = feed_name_to_idx.at(name);
    Tensor *tensor = req.add_tensor();
    int total_number = int64_data.size();

    for (uint32_t j = 0; j < int64_shape.size(); ++j) {
      tensor->add_shape(int64_shape[j]);
    }
    for (uint32_t j = 0; j < int64_lod.size(); ++j) {
      tensor->add_lod(int64_lod[j]);
    }
    tensor->set_elem_type(P_INT64);
    tensor->set_name(feed_name[idx]);
    tensor->set_alias_name(name);

    tensor->mutable_int64_data()->Resize(total_number, 0);
    memcpy(tensor->mutable_int64_data()->mutable_data(), int64_data.data(), total_number * sizeof(int64_t));
  }

  for (std::map<std::string, std::vector<int32_t>>::const_iterator iter = int32_feed_map.begin();
        iter != int32_feed_map.end();
        ++iter) {
    std::string name = iter->first;
    const std::vector<int32_t>& int32_data = iter->second;
    const std::vector<int>& int32_shape = shape_map.at(name);
    const std::vector<int>& int32_lod = lod_map.at(name);
    std::map<std::string, int>::const_iterator feed_name_it = feed_name_to_idx.find(name);
    if (feed_name_it == feed_name_to_idx.end()) {
      LOG(ERROR) << "Do not find [" << name << "] in feed_map!";
      return -1;
    }
    int idx = feed_name_to_idx.at(name);
    Tensor *tensor = req.add_tensor();
    int total_number = int32_data.size();

    for (uint32_t j = 0; j < int32_shape.size(); ++j) {
      tensor->add_shape(int32_shape[j]);
    }
    for (uint32_t j = 0; j < int32_lod.size(); ++j) {
      tensor->add_lod(int32_lod[j]);
    }
    tensor->set_elem_type(P_INT32);
    tensor->set_name(feed_name[idx]);
    tensor->set_alias_name(name);

    tensor->mutable_int_data()->Resize(total_number, 0);
    memcpy(tensor->mutable_int_data()->mutable_data(), int32_data.data(), total_number * sizeof(int32_t));
  }

  for (std::map<std::string, std::string>::const_iterator iter = string_feed_map.begin();
        iter != string_feed_map.end();
        ++iter) {
    std::string name = iter->first;
    const std::string& string_data = iter->second;
    const std::vector<int>& string_shape = shape_map.at(name);
    const std::vector<int>& string_lod = lod_map.at(name);
    std::map<std::string, int>::const_iterator feed_name_it = feed_name_to_idx.find(name);
    if (feed_name_it == feed_name_to_idx.end()) {
      LOG(ERROR) << "Do not find [" << name << "] in feed_map!";
      return -1;
    }
    int idx = feed_name_to_idx.at(name);
    Tensor *tensor = req.add_tensor();

    for (uint32_t j = 0; j < string_shape.size(); ++j) {
      tensor->add_shape(string_shape[j]);
    }
    for (uint32_t j = 0; j < string_lod.size(); ++j) {
      tensor->add_lod(string_lod[j]);
    }
    tensor->set_elem_type(P_STRING);
    tensor->set_name(feed_name[idx]);
    tensor->set_alias_name(name);

    const int string_shape_size = string_shape.size();
    // string_shape[vec_idx] = [1];cause numpy has no datatype of string.
    // we pass string via vector<vector<string> >.
    if (string_shape_size != 1) {
      LOG(ERROR) << "string_shape_size should be 1-D, but received is : "
                 << string_shape_size;
      return -1;
    }
    switch (string_shape_size) {
      case 1: {
        tensor->add_data(string_data);
        break;
      }
    }
  }
  return 0;
}

static int post_process(const Response& res,
                        std::vector<std::string>& fetch_name,
                        std::map<std::string, int>& fetch_name_to_type,
                        PredictorOutputs& outputs) {
  VLOG(2) << "get model output num";
  uint32_t model_num = res.outputs_size();
  VLOG(2) << "model num: " << model_num;
  for (uint32_t m_idx = 0; m_idx < model_num; ++m_idx) {
    VLOG(2) << "process model output index: " << m_idx;
    auto& output = res.outputs(m_idx);
    PredictorOutputs::PredictorOutput predictor_output;
    predictor_output.engine_name = output.engine_name();
    std::map<std::string, std::vector<float>>& float_data_map = *predictor_output.data.mutable_float_data_map();
    std::map<std::string, std::vector<int64_t>>& int64_data_map = *predictor_output.data.mutable_int64_data_map();
    std::map<std::string, std::vector<int32_t>>& int32_data_map = *predictor_output.data.mutable_int_data_map();
    std::map<std::string, std::string>& string_data_map = *predictor_output.data.mutable_string_data_map();
    std::map<std::string, std::vector<int>>& shape_map = *predictor_output.data.mutable_shape_map();
    std::map<std::string, std::vector<int>>& lod_map = *predictor_output.data.mutable_lod_map();

    int idx = 0;
    for (auto &name : fetch_name) {
      // int idx = _fetch_name_to_idx[name];
      int shape_size = output.tensor(idx).shape_size();
      VLOG(2) << "fetch var " << name << " index " << idx << " shape size "
              << shape_size;
      shape_map[name].resize(shape_size);
      for (int i = 0; i < shape_size; ++i) {
        shape_map[name][i] = output.tensor(idx).shape(i);
      }
      int lod_size = output.tensor(idx).lod_size();
      if (lod_size > 0) {
        lod_map[name].resize(lod_size);
        for (int i = 0; i < lod_size; ++i) {
          lod_map[name][i] = output.tensor(idx).lod(i);
        }
      }
      idx += 1;
    }
    idx = 0;

    for (auto &name : fetch_name) {
      // int idx = _fetch_name_to_idx[name];
      if (fetch_name_to_type[name] == P_INT64) {
        VLOG(2) << "ferch var " << name << "type int64";
        int size = output.tensor(idx).int64_data_size();
        int64_data_map[name] = std::vector<int64_t>(
            output.tensor(idx).int64_data().begin(),
            output.tensor(idx).int64_data().begin() + size);
      } else if (fetch_name_to_type[name] == P_FLOAT32) {
        VLOG(2) << "fetch var " << name << "type float";
        int size = output.tensor(idx).float_data_size();
        float_data_map[name] = std::vector<float>(
            output.tensor(idx).float_data().begin(),
            output.tensor(idx).float_data().begin() + size);
      } else if (fetch_name_to_type[name] == P_INT32) {
        VLOG(2) << "fetch var " << name << "type int32";
        int size = output.tensor(idx).int_data_size();
        int32_data_map[name] = std::vector<int32_t>(
            output.tensor(idx).int_data().begin(),
            output.tensor(idx).int_data().begin() + size);
      }
      idx += 1;
    }
    outputs.add_data(std::move(predictor_output));
  }
  return 0;
}

int ServingBrpcClient::predict(const PredictorInputs& inputs,
                               PredictorOutputs& outputs,
                               std::vector<std::string>& fetch_name,
                               const uint64_t log_id) {
  Timer timeline;
  int64_t preprocess_start = timeline.TimeStampUS();

  _api.thrd_initialize();
  std::string variant_tag;
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

  if (pre_process(inputs, _feed_name_to_idx, _feed_name, req) != 0) {
    LOG(ERROR) << "Failed to preprocess req!";
    return -1;
  }

  int64_t preprocess_end = timeline.TimeStampUS();
  int64_t client_infer_start = timeline.TimeStampUS();
  Response res;

  int64_t client_infer_end = 0;
  int64_t postprocess_start = 0;
  int64_t postprocess_end = 0;

  if (FLAGS_profile_client) {
    if (FLAGS_profile_server) {
      req.set_profile_server(true);
    }
  }

  res.Clear();
  if (_predictor->inference(&req, &res) != 0) {
    LOG(ERROR) << "failed call predictor with req: " << req.ShortDebugString();
    return -1;
  }

  client_infer_end = timeline.TimeStampUS();
  postprocess_start = client_infer_end;
  if (post_process(res, fetch_name, _fetch_name_to_type, outputs) != 0) {
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

  _api.thrd_clear();

  return 0;
}

}  // namespace general_model
}  // namespace paddle_serving
}  // namespace baidu
