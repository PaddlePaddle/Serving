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

#include "core/general-client/include/general_model.h"
#include <fstream>
#include "core/sdk-cpp/builtin_format.pb.h"
#include "core/sdk-cpp/include/common.h"
#include "core/sdk-cpp/include/predictor_sdk.h"

using baidu::paddle_serving::predictor::general_model::Request;
using baidu::paddle_serving::predictor::general_model::Response;
using baidu::paddle_serving::predictor::general_model::Tensor;
using baidu::paddle_serving::predictor::general_model::FeedInst;
using baidu::paddle_serving::predictor::general_model::FetchInst;

namespace baidu {
namespace paddle_serving {
namespace general_model {
using configure::GeneralModelConfig;

int PredictorClient::init(const std::string &conf_file) {
  try {
    GeneralModelConfig model_config;
    if (configure::read_proto_conf(conf_file.c_str(),
                                   &model_config) != 0) {
      LOG(ERROR) << "Failed to load general model config"
                 << ", file path: " << conf_file;
      return -1;
    }

    _feed_name_to_idx.clear();
    _fetch_name_to_idx.clear();
    _shape.clear();
    int feed_var_num = model_config.feed_var_size();
    int fetch_var_num = model_config.fetch_var_size();
    VLOG(2) << "feed var num: " << feed_var_num
            << "fetch_var_num: " << fetch_var_num;
    for (int i = 0; i < feed_var_num; ++i) {
      _feed_name_to_idx[model_config.feed_var(i).alias_name()] = i;
      VLOG(2) << "feed alias name: " << model_config.feed_var(i).alias_name()
              << " index: " << i;
      std::vector<int> tmp_feed_shape;
      VLOG(2) << "feed" << "[" << i << "] shape:";
      for (int j = 0; j < model_config.feed_var(i).shape_size(); ++j) {
        tmp_feed_shape.push_back(model_config.feed_var(i).shape(j));
        VLOG(2) << "shape[" << j << "]: "
                << model_config.feed_var(i).shape(j);
      }
      _type.push_back(model_config.feed_var(i).feed_type());
      VLOG(2) << "feed" << "[" << i << "] feed type: "
              << model_config.feed_var(i).feed_type();
      _shape.push_back(tmp_feed_shape);
    }

    for (int i = 0; i < fetch_var_num; ++i) {
      _fetch_name_to_idx[model_config.fetch_var(i).alias_name()] = i;
      VLOG(2) << "fetch [" << i << "]" << " alias name: "
              << model_config.fetch_var(i).alias_name();
      _fetch_name_to_var_name[model_config.fetch_var(i).alias_name()] =
          model_config.fetch_var(i).name();
    }
  } catch (std::exception& e) {
    LOG(ERROR) << "Failed load general model config" << e.what();
    return -1;
  }
  return 0;
}

void PredictorClient::set_predictor_conf(const std::string &conf_path,
                                         const std::string &conf_file) {
  _predictor_path = conf_path;
  _predictor_conf = conf_file;
}

int PredictorClient::destroy_predictor() {
  _api.thrd_finalize();
  _api.destroy();
}

int PredictorClient::create_predictor() {
  if (_api.create(_predictor_path.c_str(), _predictor_conf.c_str()) != 0) {
    LOG(ERROR) << "Predictor Creation Failed";
    return -1;
  }
  _api.thrd_initialize();
}

std::vector<std::vector<float>> PredictorClient::predict(
    const std::vector<std::vector<float>> &float_feed,
    const std::vector<std::string> &float_feed_name,
    const std::vector<std::vector<int64_t>> &int_feed,
    const std::vector<std::string> &int_feed_name,
    const std::vector<std::string> &fetch_name) {
  std::vector<std::vector<float>> fetch_result;
  if (fetch_name.size() == 0) {
    return fetch_result;
  }

  // we save infer_us at fetch_result[fetch_name.size()]
  fetch_result.resize(fetch_name.size() + 1);

  _api.thrd_clear();
  _predictor = _api.fetch_predictor("general_model");
  VLOG(2) << "fetch general model predictor done.";
  VLOG(2) << "float feed name size: " << float_feed_name.size();
  VLOG(2) << "int feed name size: " << int_feed_name.size();
  VLOG(2) << "fetch name size: " << fetch_name.size();
  Request req;
  for (auto & name : fetch_name) {
    req.add_fetch_var_names(name);
  }
  std::vector<Tensor *> tensor_vec;
  FeedInst *inst = req.add_insts();
  for (auto &name : float_feed_name) {
    tensor_vec.push_back(inst->add_tensor_array());
  }

  for (auto &name : int_feed_name) {
    tensor_vec.push_back(inst->add_tensor_array());
  }
  VLOG(2) << "prepare tensor vec done.";

  int vec_idx = 0;
  for (auto &name : float_feed_name) {
    int idx = _feed_name_to_idx[name];
    Tensor *tensor = tensor_vec[idx];
    for (int j = 0; j < _shape[idx].size(); ++j) {
      tensor->add_shape(_shape[idx][j]);
    }
    tensor->set_elem_type(1);
    for (int j = 0; j < float_feed[vec_idx].size(); ++j) {
      tensor->add_data(const_cast<char *>(reinterpret_cast<const char *>(
                           &(float_feed[vec_idx][j]))),
                       sizeof(float));
    }
    vec_idx++;
  }

  VLOG(2) << "feed float feed var done.";

  vec_idx = 0;
  for (auto &name : int_feed_name) {
    int idx = _feed_name_to_idx[name];
    Tensor *tensor = tensor_vec[idx];
    for (int j = 0; j < _shape[idx].size(); ++j) {
      tensor->add_shape(_shape[idx][j]);
    }
    tensor->set_elem_type(0);
    for (int j = 0; j < int_feed[vec_idx].size(); ++j) {
      tensor->add_data(const_cast<char *>(reinterpret_cast<const char *>(
                           &(int_feed[vec_idx][j]))),
                       sizeof(int64_t));
    }
    vec_idx++;
  }

  VLOG(2) << "feed int feed var done.";

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
      VLOG(2) << "fetch name: " << name;
      VLOG(2) << "tensor data size: " << len;
      fetch_result[idx].resize(len);
      for (int i = 0; i < len; ++i) {
        fetch_result[idx][i] =
            *(const float *)res.insts(0).tensor_array(idx).data(i).c_str();
      }
    }
    fetch_result[fetch_name.size()].resize(1);
    fetch_result[fetch_name.size()][0] = res.mean_infer_us();
  }

  return fetch_result;
}

std::vector<std::vector<std::vector<float>>> PredictorClient::batch_predict(
    const std::vector<std::vector<std::vector<float>>> &float_feed_batch,
    const std::vector<std::string> &float_feed_name,
    const std::vector<std::vector<std::vector<int64_t>>> &int_feed_batch,
    const std::vector<std::string> &int_feed_name,
    const std::vector<std::string> &fetch_name) {
  int batch_size = std::max(float_feed_batch.size(), int_feed_batch.size());
  std::vector<std::vector<std::vector<float>>> fetch_result_batch;
  if (fetch_name.size() == 0) {
    return fetch_result_batch;
  }
  fetch_result_batch.resize(batch_size);
  int fetch_name_num = fetch_name.size();
  for (int bi = 0; bi < batch_size; bi++) {
    fetch_result_batch[bi].resize(fetch_name_num);
  }

  _api.thrd_clear();
  _predictor = _api.fetch_predictor("general_model");
  VLOG(2) << "fetch general model predictor done.";
  VLOG(2) << "float feed name size: " << float_feed_name.size();
  VLOG(2) << "int feed name size: " << int_feed_name.size();
  Request req;
  //
  for (int bi = 0; bi < batch_size; bi++) {
    VLOG(2) << "prepare batch " << bi;
    std::vector<Tensor *> tensor_vec;
    FeedInst *inst = req.add_insts();
    std::vector<std::vector<float>> float_feed = float_feed_batch[bi];
    std::vector<std::vector<int64_t>> int_feed = int_feed_batch[bi];
    for (auto &name : float_feed_name) {
      tensor_vec.push_back(inst->add_tensor_array());
    }

    for (auto &name : int_feed_name) {
      tensor_vec.push_back(inst->add_tensor_array());
    }
    
    VLOG(2) << "batch [" << bi << "] int_feed_name and float_feed_name"
            << "prepared";
    int vec_idx = 0;
    for (auto &name : float_feed_name) {
      int idx = _feed_name_to_idx[name];
      Tensor *tensor = tensor_vec[idx];
      for (int j = 0; j < _shape[idx].size(); ++j) {
        tensor->add_shape(_shape[idx][j]);
      }
      tensor->set_elem_type(1);
      for (int j = 0; j < float_feed[vec_idx].size(); ++j) {
        tensor->add_data(const_cast<char *>(reinterpret_cast<const char *>(
                             &(float_feed[vec_idx][j]))),
                         sizeof(float));
      }
      vec_idx++;
    }

    VLOG(2) << "batch [" << bi << "] " << "float feed value prepared";

    vec_idx = 0;
    for (auto &name : int_feed_name) {
      int idx = _feed_name_to_idx[name];
      Tensor *tensor = tensor_vec[idx];
      for (int j = 0; j < _shape[idx].size(); ++j) {
        tensor->add_shape(_shape[idx][j]);
      }
      tensor->set_elem_type(0);
      VLOG(3) << "feed var name " << name << " index " << vec_idx
              << "first data " << int_feed[vec_idx][0];
      for (int j = 0; j < int_feed[vec_idx].size(); ++j) {
        tensor->add_data(const_cast<char *>(reinterpret_cast<const char *>(
                             &(int_feed[vec_idx][j]))),
                         sizeof(int64_t));
      }
      vec_idx++;
    }

    VLOG(2) << "batch [" << bi << "] " << "itn feed value prepared";
  }

  Response res;

  res.Clear();
  if (_predictor->inference(&req, &res) != 0) {
    LOG(ERROR) << "failed call predictor with req: " << req.ShortDebugString();
    exit(-1);
  } else {
    for (int bi = 0; bi < batch_size; bi++) {
      for (auto &name : fetch_name) {
        int idx = _fetch_name_to_idx[name];
        int len = res.insts(bi).tensor_array(idx).data_size();
        VLOG(2) << "fetch name: " << name;
        VLOG(2) << "tensor data size: " << len;
        fetch_result_batch[bi][idx].resize(len);
        VLOG(2)
            << "fetch name " << name << " index " << idx << " first data "
            << *(const float *)res.insts(bi).tensor_array(idx).data(0).c_str();
        for (int i = 0; i < len; ++i) {
          fetch_result_batch[bi][idx][i] =
              *(const float *)res.insts(bi).tensor_array(idx).data(i).c_str();
        }
      }
    }
  }

  return fetch_result_batch;
}

std::vector<std::vector<float>> PredictorClient::predict_with_profile(
    const std::vector<std::vector<float>> &float_feed,
    const std::vector<std::string> &float_feed_name,
    const std::vector<std::vector<int64_t>> &int_feed,
    const std::vector<std::string> &int_feed_name,
    const std::vector<std::string> &fetch_name) {
  std::vector<std::vector<float>> res;
  return res;
}

}  // namespace general_model
}  // namespace paddle_serving
}  // namespace baidu
