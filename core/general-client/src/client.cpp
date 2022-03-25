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

#include "core/general-client/include/client.h"
#include "core/sdk-cpp/include/common.h"
#include "core/sdk-cpp/general_model_service.pb.h"

namespace baidu {
namespace paddle_serving {
namespace client {
using configure::GeneralModelConfig;
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

int ServingClient::init(const std::vector<std::string>& client_conf,
           const std::string server_port) {
  if (load_client_config(client_conf) != 0) {
    LOG(ERROR) << "Failed to load client config";
    return -1;
  }

  // pure virtual func, subclass implementation
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
                                    const std::vector<int>& lod,
                                    const int datatype) {
  _float_data_map[name] = data;
  _shape_map[name] = shape;
  _lod_map[name] = lod;
  _datatype_map[name] = datatype;
}

void PredictorData::add_int64_data(const std::vector<int64_t>& data,
                                    const std::string& name,
                                    const std::vector<int>& shape,
                                    const std::vector<int>& lod,
                                    const int datatype) {
  _int64_data_map[name] = data;
  _shape_map[name] = shape;
  _lod_map[name] = lod;
  _datatype_map[name] = datatype;
}

void PredictorData::add_int32_data(const std::vector<int32_t>& data,
                                    const std::string& name,
                                    const std::vector<int>& shape,
                                    const std::vector<int>& lod,
                                    const int datatype) {
  _int32_data_map[name] = data;
  _shape_map[name] = shape;
  _lod_map[name] = lod;
  _datatype_map[name] = datatype;
}

void PredictorData::add_string_data(const std::string& data,
                                    const std::string& name,
                                    const std::vector<int>& shape,
                                    const std::vector<int>& lod,
                                    const int datatype) {
  _string_data_map[name] = data;
  _shape_map[name] = shape;
  _lod_map[name] = lod;
  _datatype_map[name] = datatype;
}

int PredictorData::get_datatype(std::string name) const {
  std::map<std::string, int>::const_iterator it = _datatype_map.find(name);
  if (it != _datatype_map.end()) {
    return it->second;
  }
  return 0;
}

void PredictorData::set_datatype(std::string name, int type) {
  _datatype_map[name] = type;
}

std::string PredictorData::print() {
  std::string res;
  res.append(map2string<std::string, float>(_float_data_map));
  res.append(map2string<std::string, int64_t>(_int64_data_map));
  res.append(map2string<std::string, int32_t>(_int32_data_map));
  res.append(map2string<std::string, std::string>(_string_data_map));
  return res;
}

int PredictorInputs::GenProto(const PredictorInputs& inputs,
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
    // default datatype = P_FLOAT32
    int datatype = inputs.get_datatype(name);
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
    tensor->set_elem_type(datatype);

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
    // default datatype = P_INT64
    int datatype = inputs.get_datatype(name);
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
    tensor->set_elem_type(datatype);
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
    // default datatype = P_INT32
    int datatype = inputs.get_datatype(name);
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
    tensor->set_elem_type(datatype);
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
    // default datatype = P_STRING
    int datatype = inputs.get_datatype(name);
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
    tensor->set_elem_type(datatype);
    tensor->set_name(feed_name[idx]);
    tensor->set_alias_name(name);

    if (datatype == P_STRING) {
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
    } else {
      tensor->set_tensor_content(string_data);
    }
    
  }
  return 0;
}

std::string PredictorOutputs::print() {
  std::string res = "";
  for (size_t i = 0; i < _datas.size(); ++i) {
    res.append(_datas[i]->engine_name);
    res.append(":");
    res.append(_datas[i]->data.print());
    res.append("\n");
  }
  return res;
}

void PredictorOutputs::clear() {
  _datas.clear();
}

int PredictorOutputs::ParseProto(const Response& res,
                                  const std::vector<std::string>& fetch_name,
                                  std::map<std::string, int>& fetch_name_to_type,
                                  PredictorOutputs& outputs) {
  VLOG(2) << "get model output num";
  uint32_t model_num = res.outputs_size();
  VLOG(2) << "model num: " << model_num;
  for (uint32_t m_idx = 0; m_idx < model_num; ++m_idx) {
    VLOG(2) << "process model output index: " << m_idx;
    auto& output = res.outputs(m_idx);
    std::shared_ptr<PredictorOutputs::PredictorOutput> predictor_output =
        std::make_shared<PredictorOutputs::PredictorOutput>();
    predictor_output->engine_name = output.engine_name();

    PredictorData& predictor_data = predictor_output->data;
    std::map<std::string, std::vector<float>>& float_data_map = *predictor_output->data.mutable_float_data_map();
    std::map<std::string, std::vector<int64_t>>& int64_data_map = *predictor_output->data.mutable_int64_data_map();
    std::map<std::string, std::vector<int32_t>>& int32_data_map = *predictor_output->data.mutable_int_data_map();
    std::map<std::string, std::string>& string_data_map = *predictor_output->data.mutable_string_data_map();
    std::map<std::string, std::vector<int>>& shape_map = *predictor_output->data.mutable_shape_map();
    std::map<std::string, std::vector<int>>& lod_map = *predictor_output->data.mutable_lod_map();

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
        VLOG(2) << "fetch var " << name << "type int64";
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
      } else if (fetch_name_to_type[name] == P_UINT8
                || fetch_name_to_type[name] == P_INT8
                || fetch_name_to_type[name] == P_FP16) {
        VLOG(2) << "fetch var [" << name << "]type="
                << fetch_name_to_type[name];
        string_data_map[name] = output.tensor(idx).tensor_content();
      }
      predictor_data.set_datatype(name, output.tensor(idx).elem_type());
      idx += 1;
    }
    outputs.add_data(predictor_output);
  }
  return 0;
}

}  // namespace client
}  // namespace paddle_serving
}  // namespace baidu
