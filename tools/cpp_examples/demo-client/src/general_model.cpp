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

#include "general_model.h"  // NOLINT
#include <fstream>
#include "sdk-cpp/builtin_format.pb.h"
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

void PredictorClient::init(const std::string &conf_file) {
  _conf_file = conf_file;
  std::ifstream fin(conf_file);
  if (!fin) {
    LOG(ERROR) << "Your inference conf file can not be found";
    exit(-1);
  }
  _feed_name_to_idx.clear();
  _fetch_name_to_idx.clear();
  _shape.clear();
  int feed_var_num = 0;
  int fetch_var_num = 0;
  fin >> feed_var_num >> fetch_var_num;
  std::string name;
  std::string fetch_var_name;
  int shape_num = 0;
  int dim = 0;
  for (int i = 0; i < feed_var_num; ++i) {
    fin >> name;
    _feed_name_to_idx[name] = i;
    fin >> shape_num;
    std::vector<int> tmp_feed_shape;
    for (int j = 0; j < shape_num; ++j) {
      fin >> dim;
      tmp_feed_shape.push_back(dim);
    }
    _shape.push_back(tmp_feed_shape);
  }

  for (int i = 0; i < fetch_var_num; ++i) {
    fin >> name;
    fin >> fetch_var_name;
    _fetch_name_to_idx[name] = i;
    _fetch_name_to_var_name[name] = fetch_var_name;
  }
}

void PredictorClient::set_predictor_conf(const std::string &conf_path,
                                         const std::string &conf_file) {
  _predictor_path = conf_path;
  _predictor_conf = conf_file;
}

int PredictorClient::create_predictor() {
  if (_api.create(_predictor_path.c_str(), _predictor_conf.c_str()) != 0) {
    LOG(ERROR) << "Predictor Creation Failed";
    return -1;
  }
  _api.thrd_initialize();
}

void PredictorClient::predict(const std::vector<std::vector<float>> &float_feed,
                              const std::vector<std::string> &float_feed_name,
                              const std::vector<std::vector<int64_t>> &int_feed,
                              const std::vector<std::string> &int_feed_name,
                              const std::vector<std::string> &fetch_name,
                              FetchedMap *fetch_result) {
  _api.thrd_clear();
  _predictor = _api.fetch_predictor("general_model");
  Request req;
  std::vector<Tensor *> tensor_vec;
  FeedInst *inst = req.add_insts();
  for (auto &name : float_feed_name) {
    tensor_vec.push_back(inst->add_tensor_array());
  }

  for (auto &name : int_feed_name) {
    tensor_vec.push_back(inst->add_tensor_array());
  }

  int vec_idx = 0;
  for (auto &name : float_feed_name) {
    int idx = _feed_name_to_idx[name];
    Tensor *tensor = tensor_vec[idx];
    for (int j = 0; j < _shape[idx].size(); ++j) {
      tensor->add_shape(_shape[idx][j]);
    }
    tensor->set_elem_type(1);
    for (int j = 0; j < float_feed[vec_idx].size(); ++j) {
      tensor->add_data((char *)(&(float_feed[vec_idx][j])),  // NOLINT
                       sizeof(float));
    }
    vec_idx++;
  }

  vec_idx = 0;
  for (auto &name : int_feed_name) {
    int idx = _feed_name_to_idx[name];
    Tensor *tensor = tensor_vec[idx];
    for (int j = 0; j < _shape[idx].size(); ++j) {
      tensor->add_shape(_shape[idx][j]);
    }
    tensor->set_elem_type(0);
    for (int j = 0; j < int_feed[vec_idx].size(); ++j) {
      tensor->add_data((char *)(&(int_feed[vec_idx][j])),  // NOLINT
                       sizeof(int64_t));
    }
    vec_idx++;
  }

  // std::map<std::string, std::vector<float> > result;
  Response res;

  res.Clear();
  if (_predictor->inference(&req, &res) != 0) {
    LOG(ERROR) << "failed call predictor with req: " << req.ShortDebugString();
    exit(-1);
  } else {
    for (auto &name : fetch_name) {
      int idx = _fetch_name_to_idx[name];
      int len = res.insts(0).tensor_array(idx).data_size();
      (*fetch_result)[name].resize(len);
      for (int i = 0; i < len; ++i) {
        (*fetch_result)[name][i] =
            *(const float *)res.insts(0).tensor_array(idx).data(i).c_str();
      }
    }
  }

  return;
}

void PredictorClient::predict_with_profile(
    const std::vector<std::vector<float>> &float_feed,
    const std::vector<std::string> &float_feed_name,
    const std::vector<std::vector<int64_t>> &int_feed,
    const std::vector<std::string> &int_feed_name,
    const std::vector<std::string> &fetch_name,
    FetchedMap *fetch_result) {
  return;
}

}  // namespace general_model
}  // namespace paddle_serving
}  // namespace baidu
