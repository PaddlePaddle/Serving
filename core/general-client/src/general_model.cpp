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
#include "core/util/include/timer.h"
DEFINE_bool(profile_client, false, "");
DEFINE_bool(profile_server, false, "");

using baidu::paddle_serving::Timer;
using baidu::paddle_serving::predictor::general_model::Request;
using baidu::paddle_serving::predictor::general_model::Response;
using baidu::paddle_serving::predictor::general_model::Tensor;
// support: FLOAT32, INT64, INT32, UINT8, INT8, FLOAT16
enum ProtoDataType {
  P_INT64 = 0,
  P_FLOAT32,
  P_INT32,
  P_FP64,
  P_INT16,
  P_FP16,
  P_BF16,
  P_UINT8,
  P_INT8,
  P_BOOL,
  P_COMPLEX64,
  P_COMPLEX128,
  P_STRING = 20,
};
std::once_flag gflags_init_flag;
namespace py = pybind11;

namespace baidu {
namespace paddle_serving {
namespace general_model {
using configure::GeneralModelConfig;

void PredictorClient::init_gflags(std::vector<std::string> argv) {
  std::call_once(gflags_init_flag, [&]() {
#ifndef BCLOUD
    FLAGS_logtostderr = true;
#endif
    argv.insert(argv.begin(), "dummy");
    int argc = argv.size();
    char **arr = new char *[argv.size()];
    std::string line;
    for (size_t i = 0; i < argv.size(); ++i) {
      arr[i] = &argv[i][0];
      line += argv[i];
      line += ' ';
    }
    google::ParseCommandLineFlags(&argc, &arr, true);
    VLOG(2) << "Init commandline: " << line;
  });
}

int PredictorClient::init(const std::vector<std::string> &conf_file) {
  try {
    GeneralModelConfig model_config;
    if (configure::read_proto_conf(conf_file[0].c_str(), &model_config) != 0) {
      LOG(ERROR) << "Failed to load general model config"
                 << ", file path: " << conf_file[0];
      return -1;
    }

    _feed_name_to_idx.clear();
    _fetch_name_to_idx.clear();
    _shape.clear();
    int feed_var_num = model_config.feed_var_size();
    _feed_name.clear();
    VLOG(2) << "feed var num: " << feed_var_num;
    for (int i = 0; i < feed_var_num; ++i) {
      _feed_name_to_idx[model_config.feed_var(i).alias_name()] = i;
      VLOG(2) << "feed [" << i << "]"
              << " name: " << model_config.feed_var(i).name();
      _feed_name.push_back(model_config.feed_var(i).name());
      VLOG(2) << "feed alias name: " << model_config.feed_var(i).alias_name()
              << " index: " << i;
      std::vector<int> tmp_feed_shape;
      VLOG(2) << "feed"
              << "[" << i << "] shape:";
      for (int j = 0; j < model_config.feed_var(i).shape_size(); ++j) {
        tmp_feed_shape.push_back(model_config.feed_var(i).shape(j));
        VLOG(2) << "shape[" << j << "]: " << model_config.feed_var(i).shape(j);
      }
      _type.push_back(model_config.feed_var(i).feed_type());
      VLOG(2) << "feed"
              << "[" << i
              << "] feed type: " << model_config.feed_var(i).feed_type();
      _shape.push_back(tmp_feed_shape);
    }

    if (conf_file.size() > 1) {
      model_config.Clear();
      if (configure::read_proto_conf(conf_file[conf_file.size() - 1].c_str(),
                                     &model_config) != 0) {
        LOG(ERROR) << "Failed to load general model config"
                   << ", file path: " << conf_file[conf_file.size() - 1];
        return -1;
      }
    }
    int fetch_var_num = model_config.fetch_var_size();
    VLOG(2) << "fetch_var_num: " << fetch_var_num;
    for (int i = 0; i < fetch_var_num; ++i) {
      _fetch_name_to_idx[model_config.fetch_var(i).alias_name()] = i;
      VLOG(2) << "fetch [" << i << "]"
              << " alias name: " << model_config.fetch_var(i).alias_name();
      _fetch_name_to_var_name[model_config.fetch_var(i).alias_name()] =
          model_config.fetch_var(i).name();
      _fetch_name_to_type[model_config.fetch_var(i).alias_name()] =
          model_config.fetch_var(i).fetch_type();
    }
  } catch (std::exception &e) {
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
  return 0;
}

int PredictorClient::create_predictor_by_desc(const std::string &sdk_desc) {
  if (_api.create(sdk_desc) != 0) {
    LOG(ERROR) << "Predictor Creation Failed";
    return -1;
  }
  // _api.thrd_initialize();
  return 0;
}

int PredictorClient::create_predictor() {
  VLOG(2) << "Predictor path: " << _predictor_path
          << " predictor file: " << _predictor_conf;
  if (_api.create(_predictor_path.c_str(), _predictor_conf.c_str()) != 0) {
    LOG(ERROR) << "Predictor Creation Failed";
    return -1;
  }
  // _api.thrd_initialize();
  return 0;
}

int PredictorClient::numpy_predict(
    const std::vector<py::array_t<float>> &float_feed,
    const std::vector<std::string> &float_feed_name,
    const std::vector<std::vector<int>> &float_shape,
    const std::vector<std::vector<int>> &float_lod_slot_batch,
    const std::vector<py::array_t<int32_t>> &int32_feed,
    const std::vector<std::string> &int32_feed_name,
    const std::vector<std::vector<int>> &int32_shape,
    const std::vector<std::vector<int>> &int32_lod_slot_batch,
    const std::vector<py::array_t<int64_t>> &int64_feed,
    const std::vector<std::string> &int64_feed_name,
    const std::vector<std::vector<int>> &int64_shape,
    const std::vector<std::vector<int>> &int64_lod_slot_batch,
    const std::vector<std::string> &string_feed,
    const std::vector<std::string> &string_feed_name,
    const std::vector<std::vector<int>> &string_shape,
    const std::vector<std::vector<int>> &string_lod_slot_batch,
    const std::vector<std::string> &fetch_name,
    PredictorRes &predict_res_batch,
    const int &pid,
    const uint64_t log_id) {
  predict_res_batch.clear();
  Timer timeline;
  int64_t preprocess_start = timeline.TimeStampUS();

  _api.thrd_initialize();
  std::string variant_tag;
  _predictor = _api.fetch_predictor("general_model", &variant_tag);
  predict_res_batch.set_variant_tag(variant_tag);
  VLOG(2) << "fetch general model predictor done.";
  VLOG(2) << "float feed name size: " << float_feed_name.size();
  VLOG(2) << "int feed name size: " << int32_feed_name.size();
  VLOG(2) << "int feed name size: " << int64_feed_name.size();
  VLOG(2) << "string feed name size: " << string_feed_name.size();
  VLOG(2) << "max body size : " << brpc::fLU64::FLAGS_max_body_size;
  Request req;
  req.set_log_id(log_id);
  for (auto &name : fetch_name) {
    req.add_fetch_var_names(name);
  }

  int vec_idx = 0;
  // batch is already in Tensor.
  std::vector<Tensor *> tensor_vec;

  for (auto &name : float_feed_name) {
    tensor_vec.push_back(req.add_tensor());
  }

  for (auto &name : int32_feed_name) {
    tensor_vec.push_back(req.add_tensor());
  }

  for (auto &name : int64_feed_name) {
    tensor_vec.push_back(req.add_tensor());
  }

  for (auto &name : string_feed_name) {
    tensor_vec.push_back(req.add_tensor());
  }

  vec_idx = 0;
  for (auto &name : float_feed_name) {
    int idx = _feed_name_to_idx[name];
    if (idx >= tensor_vec.size()) {
      LOG(ERROR) << "idx > tensor_vec.size()";
      return -1;
    }
    VLOG(2) << "prepare float feed " << name << " idx " << idx;
    int nbytes = float_feed[vec_idx].nbytes();
    void *rawdata_ptr = (void *)(float_feed[vec_idx].data(0));
    int total_number = float_feed[vec_idx].size();
    Tensor *tensor = tensor_vec[idx];

    VLOG(2) << "prepare float feed " << name << " shape size "
            << float_shape[vec_idx].size();
    for (uint32_t j = 0; j < float_shape[vec_idx].size(); ++j) {
      tensor->add_shape(float_shape[vec_idx][j]);
    }
    for (uint32_t j = 0; j < float_lod_slot_batch[vec_idx].size(); ++j) {
      tensor->add_lod(float_lod_slot_batch[vec_idx][j]);
    }
    tensor->set_elem_type(P_FLOAT32);

    tensor->set_name(_feed_name[idx]);
    tensor->set_alias_name(name);

    tensor->mutable_float_data()->Resize(total_number, 0);
    memcpy(tensor->mutable_float_data()->mutable_data(), rawdata_ptr, nbytes);
    vec_idx++;
  }

  vec_idx = 0;
  for (auto &name : int32_feed_name) {
    int idx = _feed_name_to_idx[name];
    if (idx >= tensor_vec.size()) {
      LOG(ERROR) << "idx > tensor_vec.size()";
      return -1;
    }
    Tensor *tensor = tensor_vec[idx];
    int nbytes = int32_feed[vec_idx].nbytes();
    void *rawdata_ptr = (void *)(int32_feed[vec_idx].data(0));
    int total_number = int32_feed[vec_idx].size();

    for (uint32_t j = 0; j < int32_shape[vec_idx].size(); ++j) {
      tensor->add_shape(int32_shape[vec_idx][j]);
    }
    for (uint32_t j = 0; j < int32_lod_slot_batch[vec_idx].size(); ++j) {
      tensor->add_lod(int32_lod_slot_batch[vec_idx][j]);
    }
    tensor->set_elem_type(_type[idx]);
    tensor->set_name(_feed_name[idx]);
    tensor->set_alias_name(name);

    tensor->mutable_int_data()->Resize(total_number, 0);
    memcpy(tensor->mutable_int_data()->mutable_data(), rawdata_ptr, nbytes);
    vec_idx++;
  }


  // Individual INT_64 feed data of int_input to tensor_content
  vec_idx = 0;
  for (auto &name : int64_feed_name) {
    int idx = _feed_name_to_idx[name];
    if (idx >= tensor_vec.size()) {
      LOG(ERROR) << "idx > tensor_vec.size()";
      return -1;
    }
    Tensor *tensor = tensor_vec[idx];
    int nbytes = int64_feed[vec_idx].nbytes();
    void *rawdata_ptr = (void *)(int64_feed[vec_idx].data(0));
    int total_number = int64_feed[vec_idx].size();

    for (uint32_t j = 0; j < int64_shape[vec_idx].size(); ++j) {
      tensor->add_shape(int64_shape[vec_idx][j]);
    }
    for (uint32_t j = 0; j < int64_lod_slot_batch[vec_idx].size(); ++j) {
      tensor->add_lod(int64_lod_slot_batch[vec_idx][j]);
    }
    tensor->set_elem_type(_type[idx]);
    tensor->set_name(_feed_name[idx]);
    tensor->set_alias_name(name);

    tensor->mutable_int64_data()->Resize(total_number, 0);
    memcpy(tensor->mutable_int64_data()->mutable_data(), rawdata_ptr, nbytes);
    vec_idx++;
  }

  // Add !P_STRING feed data of string_input to tensor_content
  // UINT8 INT8 FLOAT16
  vec_idx = 0;
  for (auto &name : string_feed_name) {
    int idx = _feed_name_to_idx[name];
    if (idx >= tensor_vec.size()) {
      LOG(ERROR) << "idx > tensor_vec.size()";
      return -1;
    }
    Tensor *tensor = tensor_vec[idx];

    for (uint32_t j = 0; j < string_shape[vec_idx].size(); ++j) {
      tensor->add_shape(string_shape[vec_idx][j]);
    }
    for (uint32_t j = 0; j < string_lod_slot_batch[vec_idx].size(); ++j) {
      tensor->add_lod(string_lod_slot_batch[vec_idx][j]);
    }
    tensor->set_name(_feed_name[idx]);
    tensor->set_alias_name(name);

    if (_type[idx] != P_STRING) {
      tensor->set_elem_type(_type[idx]);
      tensor->set_tensor_content(string_feed[vec_idx]);
    } else {
      tensor->set_elem_type(P_STRING);
      const int string_shape_size = string_shape[vec_idx].size();
      // string_shape[vec_idx] = [1];cause numpy has no datatype of string.
      // we pass string via vector<vector<string> >.
      if (string_shape_size != 1) {
        LOG(ERROR) << "string_shape_size should be 1-D, but received is : "
                   << string_shape_size;
        return -1;
      }
      switch (string_shape_size) {
        case 1: {
          tensor->add_data(string_feed[vec_idx]);
          break;
        }
      }
    }
    vec_idx++;
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
  } else {
    client_infer_end = timeline.TimeStampUS();
    postprocess_start = client_infer_end;
    VLOG(2) << "get model output num";
    uint32_t model_num = res.outputs_size();
    VLOG(2) << "model num: " << model_num;
    for (uint32_t m_idx = 0; m_idx < model_num; ++m_idx) {
      VLOG(2) << "process model output index: " << m_idx;
      auto output = res.outputs(m_idx);
      ModelRes model;
      model.set_engine_name(output.engine_name());
      // 在ResponseOp处，已经按照fetch_name对输出数据进行了处理
      // 所以，输出的数据与fetch_name是严格对应的，按顺序处理即可。
      for (int idx = 0; idx < output.tensor_size(); ++idx) {
        // int idx = _fetch_name_to_idx[name];
        const std::string name = output.tensor(idx).alias_name();
        model._tensor_alias_names.push_back(name);
        int shape_size = output.tensor(idx).shape_size();
        VLOG(2) << "fetch var " << name << " index " << idx << " shape size "
                << shape_size;
        model._shape_map[name].resize(shape_size);
        for (int i = 0; i < shape_size; ++i) {
          model._shape_map[name][i] = output.tensor(idx).shape(i);
        }
        int lod_size = output.tensor(idx).lod_size();
        if (lod_size > 0) {
          model._lod_map[name].resize(lod_size);
          for (int i = 0; i < lod_size; ++i) {
            model._lod_map[name][i] = output.tensor(idx).lod(i);
          }
        }

        if (_fetch_name_to_type[name] == P_INT64) {
          VLOG(2) << "ferch var " << name << "type int64";
          int size = output.tensor(idx).int64_data_size();
          model._int64_value_map[name] = std::vector<int64_t>(
              output.tensor(idx).int64_data().begin(),
              output.tensor(idx).int64_data().begin() + size);
        } else if (_fetch_name_to_type[name] == P_FLOAT32) {
          VLOG(2) << "fetch var " << name << "type float";
          int size = output.tensor(idx).float_data_size();
          model._float_value_map[name] = std::vector<float>(
              output.tensor(idx).float_data().begin(),
              output.tensor(idx).float_data().begin() + size);
        } else if (_fetch_name_to_type[name] == P_INT32) {
          VLOG(2) << "fetch var " << name << "type int32";
          int size = output.tensor(idx).int_data_size();
          model._int32_value_map[name] = std::vector<int32_t>(
              output.tensor(idx).int_data().begin(),
              output.tensor(idx).int_data().begin() + size);
        } else if (_fetch_name_to_type[name] == P_UINT8) {
          VLOG(2) << "fetch var " << name << "type uint8";
          model._string_value_map[name] = output.tensor(idx).tensor_content();
        } else if (_fetch_name_to_type[name] == P_INT8) {
          VLOG(2) << "fetch var " << name << "type int8";
          model._string_value_map[name] = output.tensor(idx).tensor_content();
        } else if (_fetch_name_to_type[name] == P_FP16) {
          VLOG(2) << "fetch var " << name << "type float16";
          model._string_value_map[name] = output.tensor(idx).tensor_content();
        }
      }
      predict_res_batch.add_model_res(std::move(model));
    }
    postprocess_end = timeline.TimeStampUS();
  }

  if (FLAGS_profile_client) {
    std::ostringstream oss;
    oss << "PROFILE\t"
        << "pid:" << pid << "\t"
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
