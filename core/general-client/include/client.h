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

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <memory>

namespace baidu {
namespace paddle_serving {
namespace predictor {
  namespace general_model {
    class Request;
    class Response;
  }
}
namespace client {

class PredictorInputs;
class PredictorOutputs;

class ServingClient {
 public:
  ServingClient() {};

  virtual ~ServingClient() = default;

  int init(const std::vector<std::string>& client_conf,
           const std::string server_port);

  int load_client_config(const std::vector<std::string>& client_conf);

  virtual int connect(const std::string server_port) = 0;

  virtual int predict(const PredictorInputs& inputs,
                      PredictorOutputs& outputs,
                      std::vector<std::string>& fetch_name,
                      const uint64_t log_id) = 0;

 protected:
  std::map<std::string, int> _feed_name_to_idx;
  std::vector<std::string> _feed_name;
  std::map<std::string, int> _fetch_name_to_idx;
  std::map<std::string, std::string> _fetch_name_to_var_name;
  std::map<std::string, int> _fetch_name_to_type;
  std::vector<std::vector<int>> _shape;
  std::vector<int> _type;
  std::vector<int64_t> _last_request_ts;
};

class PredictorData {
 public:
  PredictorData() {};
  virtual ~PredictorData() {};

  virtual void add_float_data(const std::vector<float>& data,
                              const std::string& name,
                              const std::vector<int>& shape,
                              const std::vector<int>& lod);

  virtual void add_int64_data(const std::vector<int64_t>& data,
                              const std::string& name,
                              const std::vector<int>& shape,
                              const std::vector<int>& lod);

  virtual void add_int32_data(const std::vector<int32_t>& data,
                              const std::string& name,
                              const std::vector<int>& shape,
                              const std::vector<int>& lod);

  virtual void add_string_data(const std::string& data,
                               const std::string& name,
                               const std::vector<int>& shape,
                               const std::vector<int>& lod);

  virtual const std::map<std::string, std::vector<float>>& float_data_map() const {
    return _float_data_map;
  };

  virtual std::map<std::string, std::vector<float>>* mutable_float_data_map() {
    return &_float_data_map;
  };

  virtual const std::map<std::string, std::vector<int64_t>>& int64_data_map() const {
    return _int64_data_map;
  };

  virtual std::map<std::string, std::vector<int64_t>>* mutable_int64_data_map() {
    return &_int64_data_map;
  };

  virtual const std::map<std::string, std::vector<int32_t>>& int_data_map() const {
    return _int32_data_map;
  };

  virtual std::map<std::string, std::vector<int32_t>>* mutable_int_data_map() {
    return &_int32_data_map;
  };

  virtual const std::map<std::string, std::string>& string_data_map() const {
    return _string_data_map;
  };

  virtual std::map<std::string, std::string>* mutable_string_data_map() {
    return &_string_data_map;
  };

  virtual const std::map<std::string, std::vector<int>>& shape_map() const {
    return _shape_map;
  };

  virtual std::map<std::string, std::vector<int>>* mutable_shape_map() {
    return &_shape_map;
  };

  virtual const std::map<std::string, std::vector<int>>& lod_map() const {
    return _lod_map;
  };

  virtual std::map<std::string, std::vector<int>>* mutable_lod_map() {
    return &_lod_map;
  };

  virtual std::string print();

 private:
  template<typename T1, typename T2>
  std::string map2string(const std::map<T1, std::vector<T2>>& map) {
    std::ostringstream oss;
    oss.str("");
    oss.precision(6);
	  oss.setf(std::ios::fixed);
    std::string key_seg = ":";
    std::string val_seg = ",";
    std::string end_seg = "\n";
    typename std::map<T1, std::vector<T2>>::const_iterator it = map.begin();
    typename std::map<T1, std::vector<T2>>::const_iterator itEnd = map.end();
    for (; it != itEnd; it++) {
      oss << "{";
      oss << it->first << key_seg;
      const std::vector<T2>& v = it->second;
      for (size_t i = 0; i < v.size(); ++i) {
        if (i != v.size() - 1) {
          oss << v[i] << val_seg;
        }
        else {
          oss << v[i];
        }
      }
      oss << "}";
    }
    return oss.str();
  };

  template<typename T1, typename T2>
  std::string map2string(const std::map<T1, T2>& map) {
    std::ostringstream oss;
    oss.str("");
    std::string key_seg = ":";
    std::string val_seg = ",";
    std::string end_seg = "\n";
    typename std::map<T1, T2>::const_iterator it = map.begin();
    typename std::map<T1, T2>::const_iterator itEnd = map.end();
    for (; it != itEnd; it++) {
      oss << "{";
      oss << it->first << key_seg << it->second;
      oss << "}";
    }
    return oss.str();
  };

 protected:
  std::map<std::string, std::vector<float>> _float_data_map;
  std::map<std::string, std::vector<int64_t>> _int64_data_map;
  std::map<std::string, std::vector<int32_t>> _int32_data_map;
  std::map<std::string, std::string> _string_data_map;
  std::map<std::string, std::vector<int>> _shape_map;
  std::map<std::string, std::vector<int>> _lod_map;
};

class PredictorInputs : public PredictorData {
 public:
  PredictorInputs() {};
  virtual ~PredictorInputs() {};

  static int gen_proto(const PredictorInputs& inputs,
                      const std::map<std::string, int>& feed_name_to_idx,
                      const std::vector<std::string>& feed_name,
                      predictor::general_model::Request& req);
};

class PredictorOutputs {
 public:
  struct PredictorOutput {
    std::string engine_name;
    PredictorData data;
  };

  PredictorOutputs() {};
  virtual ~PredictorOutputs() {};

  virtual std::vector<std::shared_ptr<PredictorOutputs::PredictorOutput>>& datas() {
    return _datas;
  };

  virtual std::vector<std::shared_ptr<PredictorOutputs::PredictorOutput>>* mutable_datas() {
    return &_datas;
  };

  virtual void add_data(const std::shared_ptr<PredictorOutputs::PredictorOutput>& data) {
    _datas.push_back(data);
  };

  virtual std::string print();

  virtual void clear();

  static int parse_proto(const predictor::general_model::Response& res,
                        const std::vector<std::string>& fetch_name,
                        std::map<std::string, int>& fetch_name_to_type,
                        PredictorOutputs& outputs);

 protected:
  std::vector<std::shared_ptr<PredictorOutputs::PredictorOutput>> _datas;
};

}  // namespace client
}  // namespace paddle_serving
}  // namespace baidu