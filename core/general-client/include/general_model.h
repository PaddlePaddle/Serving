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

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "core/sdk-cpp/builtin_format.pb.h"
#include "core/sdk-cpp/general_model_service.pb.h"
#include "core/sdk-cpp/include/common.h"
#include "core/sdk-cpp/include/predictor_sdk.h"

using baidu::paddle_serving::sdk_cpp::Predictor;
using baidu::paddle_serving::sdk_cpp::PredictorApi;

// given some input data, pack into pb, and send request
namespace baidu {
namespace paddle_serving {
namespace general_model {

typedef std::map<std::string, std::vector<float>> FetchedMap;

typedef std::map<std::string, std::vector<std::vector<float>>> BatchFetchedMap;

class PredictorClient {
 public:
  PredictorClient() {}
  ~PredictorClient() {}

  int init(const std::string& client_conf);

  void set_predictor_conf(const std::string& conf_path,
                          const std::string& conf_file);

  int create_predictor_by_desc(const std::string & sdk_desc);

  int create_predictor();
  int destroy_predictor();

  std::vector<std::vector<float>> predict(
      const std::vector<std::vector<float>>& float_feed,
      const std::vector<std::string>& float_feed_name,
      const std::vector<std::vector<int64_t>>& int_feed,
      const std::vector<std::string>& int_feed_name,
      const std::vector<std::string>& fetch_name);

  std::vector<std::vector<std::vector<float>>> batch_predict(
      const std::vector<std::vector<std::vector<float>>>& float_feed_batch,
      const std::vector<std::string>& float_feed_name,
      const std::vector<std::vector<std::vector<int64_t>>>& int_feed_batch,
      const std::vector<std::string>& int_feed_name,
      const std::vector<std::string>& fetch_name);

  std::vector<std::vector<float>> predict_with_profile(
      const std::vector<std::vector<float>>& float_feed,
      const std::vector<std::string>& float_feed_name,
      const std::vector<std::vector<int64_t>>& int_feed,
      const std::vector<std::string>& int_feed_name,
      const std::vector<std::string>& fetch_name);

 private:
  PredictorApi _api;
  Predictor* _predictor;
  std::string _predictor_conf;
  std::string _predictor_path;
  std::string _conf_file;
  std::map<std::string, int> _feed_name_to_idx;
  std::map<std::string, int> _fetch_name_to_idx;
  std::map<std::string, std::string> _fetch_name_to_var_name;
  std::vector<std::vector<int>> _shape;
  std::vector<int> _type;
};

}  // namespace general_model
}  // namespace paddle_serving
}  // namespace baidu

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
