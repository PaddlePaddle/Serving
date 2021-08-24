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

#include "core/general-client/include/client.h"
#include "core/sdk-cpp/include/common.h"

namespace baidu {
namespace paddle_serving {
namespace client {
using configure::GeneralModelConfig;

int ServingClient::init(const std::vector<std::string>& client_conf,
           const std::string server_port) {
  if (load_client_config(client_conf) != 0) {
    LOG(ERROR) << "Failed to load client config";
    return -1;
  }

  if (connect(server_port) != 0) {
    LOG(ERROR) << "Failed to connect";
    return -1;
  }

  return 0;
}

int ServingClient::load_client_config(const std::vector<std::string> &conf_file) {
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

void PredictorData::add_float_data(const std::vector<float>& data,
                                    const std::string& name,
                                    const std::vector<int>& shape,
                                    const std::vector<int>& lod) {
  _float_data_map[name] = data;
  _shape_map[name] = shape;
  _lod_map[name] = lod;
}

void PredictorData::add_int64_data(const std::vector<int64_t>& data,
                                    const std::string& name,
                                    const std::vector<int>& shape,
                                    const std::vector<int>& lod) {
  _int64_data_map[name] = data;
  _shape_map[name] = shape;
  _lod_map[name] = lod;
}

void PredictorData::add_int32_data(const std::vector<int32_t>& data,
                                    const std::string& name,
                                    const std::vector<int>& shape,
                                    const std::vector<int>& lod) {
  _int32_data_map[name] = data;
  _shape_map[name] = shape;
  _lod_map[name] = lod;
}

void PredictorData::add_string_data(const std::string& data,
                                    const std::string& name,
                                    const std::vector<int>& shape,
                                    const std::vector<int>& lod) {
  _string_data_map[name] = data;
  _shape_map[name] = shape;
  _lod_map[name] = lod;
}

}  // namespace general_model
}  // namespace paddle_serving
}  // namespace baidu
