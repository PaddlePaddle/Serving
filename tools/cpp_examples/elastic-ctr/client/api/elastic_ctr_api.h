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

#define BRPC_WITH_GLOG 1  // To make sure prpoer glog inclusion

#include <map>
#include <set>
#include <string>
#include <vector>
#include "elastic-ctr/client/elastic_ctr_prediction.pb.h"
#include "sdk-cpp/include/predictor_sdk.h"

using baidu::paddle_serving::sdk_cpp::Predictor;
using baidu::paddle_serving::sdk_cpp::PredictorApi;
using baidu::paddle_serving::predictor::elastic_ctr::Slot;
using baidu::paddle_serving::predictor::elastic_ctr::Request;
using baidu::paddle_serving::predictor::elastic_ctr::ReqInstance;
using baidu::paddle_serving::predictor::elastic_ctr::Response;
using baidu::paddle_serving::predictor::elastic_ctr::ResInstance;

namespace baidu {
namespace paddle_serving {
namespace elastic_ctr {

struct InstanceInfo {
  std::map<std::string, Slot *> slot_map_;
};

class ThreadResource {
 public:
  int clear();

  Request *get_request() { return &request_; }

  Response *get_response() { return &response_; }

  ReqInstance *add_instance();
  int add_slot(ReqInstance *instance,
               const std::string &slot_name,
               uint64_t value);
  void validate_request(const std::set<std::string> &slot_names);

 private:
  Request request_;
  Response response_;

  std::map<ReqInstance *, InstanceInfo *> instance_map_;
};

struct Prediction {
  float prob0;
  float prob1;
};

class ElasticCTRPredictorApi {
 public:
  ElasticCTRPredictorApi() {}

  int init(const char *path,
           const char *slot_conf_file,
           const char *serving_conf_file);

  int thrd_initialize();

  int thrd_clear();

  int thrd_finalize();

  void destroy();

  static ElasticCTRPredictorApi &instance() {
    static ElasticCTRPredictorApi api;
    return api;
  }

 public:
  ReqInstance *add_instance();
  int add_slot(ReqInstance *instance,
               const std::string slot_name,
               int64_t value);
  int inference(std::vector<std::vector<float>> &results_vec);  // NOLINT

 private:
  static int read_slot_conf(const char *path, const char *slot_conf_file);
  void validate_request();

 private:
  PredictorApi api_;
  pthread_key_t tls_bspec_key_;
  static std::set<std::string> slot_names_;
};

}  // namespace elastic_ctr
}  // namespace paddle_serving
}  // namespace baidu
