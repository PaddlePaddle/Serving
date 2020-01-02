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

#include <fstream>
#include "general_model.h"
#include "sdk-cpp/builtin_format.pb.h"
#include "sdk-cpp/load_general_model_service.pb.h"
#include "sdk-cpp/include/common.h"
#include "sdk-cpp/include/predictor_sdk.h"

using baidu::paddle_serving::predictor::general_model::Request;
using baidu::paddle_serving::predictor::general_model::Response;
using baidu::paddle_serving::predictor::general_model::Tensor;
using baidu::paddle_serving::predictor::general_model::FeedInst;
using baidu::paddle_serving::predictor::general_model::FetchInst;

namespace baidu {
namespace paddle_serving {
namespace general_model {

void PredictorClient::connect(const std::vector<std::string> & ep_list) {
  _eplist = ep_list;
}

FetchedMap & PredictorClient::predict(
    const std::vector<std::vector<float> > & float_feed,
    const std::vector<std::string> & float_feed_name,
    const std::vector<std::vector<int64_t> > & int_feed,
    const std::vector<std::string> & int_feed_name,
    const std::vector<std::string> & fetch_name) {
  Request req;
  std::vector<Tensor *> tensor_vec;
  FeedInst * inst = req.add_insts();
  for (auto & name : float_feed_name) {
    tensor_vec.push_back(inst->add_tensor_array());
  }

  for (auto & name : int_feed_name) {
    tensor_vec.push_back(inst->add_tensor_array());
  }

  int vec_idx = 0;
  for (auto & name : float_feed_name) {
    int idx = _feed_name_to_idx[name];
    Tensor * tensor = tensor_vec[idx];
    for (int j = 0; j < _shape[idx].size(); ++j) {
      tensor->add_shape(_shape[idx][j]);
    }
    tensor->set_elem_type(1);
    tensor->mutable_data()->Reserve(
        float_feed[vec_idx].size() * sizeof(float));
    void * dst_ptr = tensor->mutable_data()->mutable_data();
    memcpy(dst_ptr, float_feed[vec_idx].data(),
           float_feed[vec_idx].size() * sizeof(float));
    vec_idx++;
  }

  vec_idx = 0;
  for (auto & name : int_feed_name) {
    int idx = _feed_name_to_idx[name];
    Tensor * tensor = tensor_vec[idx];
    for (int j = 0; j < _shape[idx].size(); ++j) {
      tensor->add_shape(_shape[idx][j]);
    }
    tensor->set_elem_type(0);
    tensor->mutable_data()->Reserve(
        int_feed[vec_idx].size() * sizeof(int64_t));
    void * dst_ptr = tensor->mutable_data()->mutable_data();
    memcpy(dst_ptr, int_feed[vec_idx].data(),
           int_feed[idx].size() * sizeof(int64_t));
  }

  std::map<std::string, std::vector<float> > result;
  Response res;
  if (_predictor->inference(&req, &res) != 0) {
    FetchInst * inst = res.add_insts();
    for (auto & name : fetch_name) {
      int idx = _fetch_name_to_idx[name];
      result[name].resize(inst->tensor_array(idx).data_size() / sizeof(float));
      memcpy(result[name].data(),
             inst->mutable_tensor_array(idx)->mutable_data(),
             inst->tensor_array(idx).data_size() / sizeof(float));
    }
  }

  return result;
}

FetchedMap & PredictorClient::predict_with_profile(
    const std::vector<std::vector<float> > & float_feed,
    const std::vector<std::string> & float_feed_name,
    const std::vector<std::vector<int64_t> > & int_feed,
    const std::vector<std::string> & int_feed_name,
    const std::vector<std::string> & fetch_name) {
  FetchedMap res;
  return res;
}

}  // namespace general_model
}  // namespace paddle_serving
}  // namespace baidu
