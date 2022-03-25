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

#include <pybind11/numpy.h>
#include <algorithm>
#include <fstream>
#include <map>
#include <string>
#include <utility>  // move
#include <vector>
#include "core/sdk-cpp/builtin_format.pb.h"
#include "core/sdk-cpp/general_model_service.pb.h"
#include "core/sdk-cpp/include/common.h"
#include "core/sdk-cpp/include/predictor_sdk.h"
using baidu::paddle_serving::sdk_cpp::Predictor;
using baidu::paddle_serving::sdk_cpp::PredictorApi;

DECLARE_bool(profile_client);
DECLARE_bool(profile_server);

// given some input data, pack into pb, and send request
namespace py = pybind11;
namespace baidu {
namespace paddle_serving {
namespace general_model {

class ModelRes {
 public:
  ModelRes() {}
  ModelRes(const ModelRes& res) {
    _engine_name = res._engine_name;
    _int64_value_map.insert(res._int64_value_map.begin(),
                            res._int64_value_map.end());
    _float_value_map.insert(res._float_value_map.begin(),
                            res._float_value_map.end());
    _int32_value_map.insert(res._int32_value_map.begin(),
                            res._int32_value_map.end());
    _string_value_map.insert(res._string_value_map.begin(),
                            res._string_value_map.end());
    _shape_map.insert(res._shape_map.begin(), res._shape_map.end());
    _lod_map.insert(res._lod_map.begin(), res._lod_map.end());
    _tensor_alias_names.insert(_tensor_alias_names.end(),
                               res._tensor_alias_names.begin(),
                               res._tensor_alias_names.end());
  }
  ModelRes(ModelRes&& res) {
    _engine_name = std::move(res._engine_name);
    _int64_value_map.insert(
        std::make_move_iterator(std::begin(res._int64_value_map)),
        std::make_move_iterator(std::end(res._int64_value_map)));
    _float_value_map.insert(
        std::make_move_iterator(std::begin(res._float_value_map)),
        std::make_move_iterator(std::end(res._float_value_map)));
    _int32_value_map.insert(
        std::make_move_iterator(std::begin(res._int32_value_map)),
        std::make_move_iterator(std::end(res._int32_value_map)));
    _string_value_map.insert(
        std::make_move_iterator(std::begin(res._string_value_map)),
        std::make_move_iterator(std::end(res._string_value_map)));
    _shape_map.insert(std::make_move_iterator(std::begin(res._shape_map)),
                      std::make_move_iterator(std::end(res._shape_map)));
    _lod_map.insert(std::make_move_iterator(std::begin(res._lod_map)),
                    std::make_move_iterator(std::end(res._lod_map)));
    _tensor_alias_names.insert(
        _tensor_alias_names.end(),
        std::make_move_iterator(std::begin(res._tensor_alias_names)),
        std::make_move_iterator(std::end(res._tensor_alias_names)));
  }
  ~ModelRes() {}
  const std::vector<int64_t>& get_int64_by_name(const std::string& name) {
    return _int64_value_map[name];
  }
  std::vector<int64_t>&& get_int64_by_name_with_rv(const std::string& name) {
    return std::move(_int64_value_map[name]);
  }
  const std::vector<float>& get_float_by_name(const std::string& name) {
    return _float_value_map[name];
  }
  std::vector<float>&& get_float_by_name_with_rv(const std::string& name) {
    return std::move(_float_value_map[name]);
  }
  const std::vector<int32_t>& get_int32_by_name(const std::string& name) {
    return _int32_value_map[name];
  }
  std::vector<int32_t>&& get_int32_by_name_with_rv(const std::string& name) {
    return std::move(_int32_value_map[name]);
  }
  const std::string& get_string_by_name(const std::string& name) {
    return _string_value_map[name];
  }
  std::string&& get_string_by_name_with_rv(const std::string& name) {
    return std::move(_string_value_map[name]);
  }
  const std::vector<int>& get_shape_by_name(const std::string& name) {
    return _shape_map[name];
  }
  std::vector<int>&& get_shape_by_name_with_rv(const std::string& name) {
    return std::move(_shape_map[name]);
  }
  const std::vector<int>& get_lod_by_name(const std::string& name) {
    return _lod_map[name];
  }
  std::vector<int>&& get_lod_by_name_with_rv(const std::string& name) {
    return std::move(_lod_map[name]);
  }
  void set_engine_name(const std::string& engine_name) {
    _engine_name = engine_name;
  }
  const std::string& engine_name() { return _engine_name; }

  const std::vector<std::string>& tensor_alias_names() {
    return _tensor_alias_names;
  }
  ModelRes& operator=(ModelRes&& res) {
    if (this != &res) {
      _engine_name = std::move(res._engine_name);
      _int64_value_map.insert(
          std::make_move_iterator(std::begin(res._int64_value_map)),
          std::make_move_iterator(std::end(res._int64_value_map)));
      _float_value_map.insert(
          std::make_move_iterator(std::begin(res._float_value_map)),
          std::make_move_iterator(std::end(res._float_value_map)));
      _int32_value_map.insert(
          std::make_move_iterator(std::begin(res._int32_value_map)),
          std::make_move_iterator(std::end(res._int32_value_map)));
      _string_value_map.insert(
          std::make_move_iterator(std::begin(res._string_value_map)),
          std::make_move_iterator(std::end(res._string_value_map)));
      _shape_map.insert(std::make_move_iterator(std::begin(res._shape_map)),
                        std::make_move_iterator(std::end(res._shape_map)));
      _lod_map.insert(std::make_move_iterator(std::begin(res._lod_map)),
                      std::make_move_iterator(std::end(res._lod_map)));
      _tensor_alias_names.insert(
          _tensor_alias_names.end(),
          std::make_move_iterator(std::begin(res._tensor_alias_names)),
          std::make_move_iterator(std::end(res._tensor_alias_names)));
    }
    return *this;
  }

 public:
  std::string _engine_name;
  std::map<std::string, std::vector<int64_t>> _int64_value_map;
  std::map<std::string, std::vector<float>> _float_value_map;
  std::map<std::string, std::vector<int32_t>> _int32_value_map;
  std::map<std::string, std::string> _string_value_map;
  std::map<std::string, std::vector<int>> _shape_map;
  std::map<std::string, std::vector<int>> _lod_map;
  std::vector<std::string> _tensor_alias_names;
};

class PredictorRes {
 public:
  PredictorRes() {}
  ~PredictorRes() {}

 public:
  void clear() {
    _models.clear();
    _engine_names.clear();
  }
  const std::vector<int64_t>& get_int64_by_name(const int model_idx,
                                                const std::string& name) {
    return _models[model_idx].get_int64_by_name(name);
  }
  std::vector<int64_t>&& get_int64_by_name_with_rv(const int model_idx,
                                                   const std::string& name) {
    return std::move(_models[model_idx].get_int64_by_name_with_rv(name));
  }
  const std::vector<float>& get_float_by_name(const int model_idx,
                                              const std::string& name) {
    return _models[model_idx].get_float_by_name(name);
  }
  std::vector<float>&& get_float_by_name_with_rv(const int model_idx,
                                                 const std::string& name) {
    return std::move(_models[model_idx].get_float_by_name_with_rv(name));
  }
  const std::vector<int32_t>& get_int32_by_name(const int model_idx,
                                                const std::string& name) {
    return _models[model_idx].get_int32_by_name(name);
  }
  std::vector<int32_t>&& get_int32_by_name_with_rv(const int model_idx,
                                                   const std::string& name) {
    return std::move(_models[model_idx].get_int32_by_name_with_rv(name));
  }
  const std::string& get_string_by_name(const int model_idx,
                                                const std::string& name) {
    return _models[model_idx].get_string_by_name(name);
  }
  std::string&& get_string_by_name_with_rv(const int model_idx,
                                                   const std::string& name) {
    return std::move(_models[model_idx].get_string_by_name_with_rv(name));
  }
  const std::vector<int>& get_shape_by_name(const int model_idx,
                                            const std::string& name) {
    return _models[model_idx].get_shape_by_name(name);
  }
  const std::vector<int>&& get_shape_by_name_with_rv(const int model_idx,
                                                     const std::string& name) {
    return std::move(_models[model_idx].get_shape_by_name_with_rv(name));
  }
  const std::vector<int>& get_lod_by_name(const int model_idx,
                                          const std::string& name) {
    return _models[model_idx].get_lod_by_name(name);
  }
  const std::vector<int>&& get_lod_by_name_with_rv(const int model_idx,
                                                   const std::string& name) {
    return std::move(_models[model_idx].get_lod_by_name_with_rv(name));
  }
  void add_model_res(ModelRes&& res) {
    _engine_names.push_back(res.engine_name());
    _models.emplace_back(std::move(res));
  }
  void set_variant_tag(const std::string& variant_tag) {
    _variant_tag = variant_tag;
  }
  const std::string& variant_tag() { return _variant_tag; }
  const std::vector<std::string>& get_engine_names() { return _engine_names; }
  const std::vector<std::string>& get_tensor_alias_names(const int model_idx) {
    _tensor_alias_names = _models[model_idx].tensor_alias_names();
    return _tensor_alias_names;
  }

 private:
  std::vector<ModelRes> _models;
  std::string _variant_tag;
  std::vector<std::string> _engine_names;
  std::vector<std::string> _tensor_alias_names;
};

class PredictorClient {
 public:
  PredictorClient() {}
  ~PredictorClient() {}

  void init_gflags(std::vector<std::string> argv);

  int init(const std::vector<std::string>& client_conf);

  void set_predictor_conf(const std::string& conf_path,
                          const std::string& conf_file);

  int create_predictor_by_desc(const std::string& sdk_desc);

  int create_predictor();

  int destroy_predictor();

  int numpy_predict(const std::vector<py::array_t<float>>& float_feed,
                    const std::vector<std::string>& float_feed_name,
                    const std::vector<std::vector<int>>& float_shape,
                    const std::vector<std::vector<int>>& float_lod_slot_batch,
                    const std::vector<py::array_t<int32_t>> &int32_feed,
                    const std::vector<std::string> &int32_feed_name,
                    const std::vector<std::vector<int>> &int32_shape,
                    const std::vector<std::vector<int>> &int32_lod_slot_batch,
                    const std::vector<py::array_t<int64_t>> &int64_feed,
                    const std::vector<std::string> &int64_feed_name,
                    const std::vector<std::vector<int>> &int64_shape,
                    const std::vector<std::vector<int>> &int64_lod_slot_batch,
                    const std::vector<std::string>& string_feed,
                    const std::vector<std::string>& string_feed_name,
                    const std::vector<std::vector<int>>& string_shape,
                    const std::vector<std::vector<int>>& string_lod_slot_batch,
                    const std::vector<std::string>& fetch_name,
                    PredictorRes& predict_res_batch,  // NOLINT
                    const int& pid,
                    const uint64_t log_id);

 private:
  PredictorApi _api;
  Predictor* _predictor;
  std::string _predictor_conf;
  std::string _predictor_path;
  std::string _conf_file;
  std::map<std::string, int> _feed_name_to_idx;
  std::vector<std::string> _feed_name;
  std::map<std::string, int> _fetch_name_to_idx;
  std::map<std::string, std::string> _fetch_name_to_var_name;
  std::map<std::string, int> _fetch_name_to_type;
  std::vector<std::vector<int>> _shape;
  std::vector<int> _type;
  std::vector<int64_t> _last_request_ts;
};

}  // namespace general_model
}  // namespace paddle_serving
}  // namespace baidu

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
