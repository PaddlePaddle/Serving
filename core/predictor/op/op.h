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
#include <bvar/bvar.h>  // bvar::LatencyRecorder
#include <cstdlib>
#include <string>
#include <vector>
#include "core/predictor/common/inner_common.h"
#include "core/predictor/framework/channel.h"
#include "core/predictor/framework/op_repository.h"
#include "core/predictor/framework/predictor_metric.h"  // PredictorMetric

namespace baidu {
namespace paddle_serving {
namespace predictor {

class Dag;

class Op {
 public:
  Op()
      : _bus(NULL),
        _dag(NULL),
        _has_calc(false),
        _has_init(false),
        _timer(NULL) {}

  virtual ~Op() {}

  // ------Getters for Channel/Data/Message of dependent OP-----

  // Get the Channel object of dependent OP
  Channel* mutable_depend_channel(const std::string& op);

  // Get the Channel object of dependent OP
  const Channel* get_depend_channel(const std::string& op) const;

  template <typename T>
  T* mutable_depend_argument(const std::string& op) {
    Channel* channel = mutable_depend_channel(op);
    if (channel == NULL) {
      LOG(WARNING) << "cannot mutable channel of " << op << " in " << _name;
      return NULL;
    }

    OpChannel<T>* op_channel = dynamic_cast<OpChannel<T>*>(channel);
    if (!op_channel) {
      LOG(ERROR) << "Cannot dynamic cast channel of op:" << this->name()
                 << " to type: " << typeid(T).name();
      return NULL;
    }

    return op_channel->data();
  }

  template <typename T>
  const T* get_depend_argument(const std::string& op) const {
    const Channel* channel = get_depend_channel(op);
    if (channel == NULL) {
      LOG(WARNING) << "cannot get read-only channel of " << op << " in "
                   << _name;
      return NULL;
    }

    const OpChannel<T>* op_channel = dynamic_cast<const OpChannel<T>*>(channel);
    if (!op_channel) {
      LOG(ERROR) << "Cannot dynamic cast channel of op:" << this->name()
                 << " to type: " << typeid(T).name();
      return NULL;
    }

    return op_channel->data();
  }

  // -----Getters for Channel/Data/Message of current OP----

  // Get pointer to the progobuf message of current OP
  google::protobuf::Message* mutable_message();

  // Get pointer to the protobuf message of current OP
  const google::protobuf::Message* get_message() const;

  // Get the template class data object of current OP
  template <typename T>
  T* mutable_data() {
    Channel* channel = mutable_channel();
    return (dynamic_cast<OpChannel<T>*>(channel))->data();
  }

  // Get the template class data object of current OP
  template <typename T>
  const T* get_data() const {
    const Channel* channel = get_channel();
    return (dynamic_cast<const OpChannel<T>*>(channel))->data();
  }

  // ---------------- Other base class members ----------------

  int init(Bus* bus,
           Dag* dag,
           uint32_t id,
           const std::string& name,
           const std::string& type,
           void* conf,
           const std::vector<std::string>& address,
           const uint64_t log_id);

  int deinit();

  int check_time(const char* tag);

  int process(const uint64_t log_id, bool debug);

  std::string time_info();

  // Get the input object
  const google::protobuf::Message* get_request_message();

  bool has_calc();

  const char* name() const;

  const std::string& op_name() const { return _name; }

  const std::string& full_name() const { return _full_name; }

  const std::vector<std::string>& address() const { return _address; }

  const std::vector<std::string>& pre_names() const { return _pre_node_names; }

  void set_full_name(const std::string full_name) { _full_name = full_name; }

  void add_pre_node_name(const std::string pre_name) {
    _pre_node_names.push_back(pre_name);
  }

  const std::string& type() const;

  uint32_t id() const;

  // Set the name of the Op as the key of the matching engine.
  // Notes that this key is only used by infer_op (only the
  // infer_op needs to find the corresponding engine).
  // At present, there is only general_infer_op.
  void set_engine_name(const std::string engine_name) {
    _engine_name = engine_name;
  }

  const std::string& engine_name() const { return _engine_name; }

  // --------------- Default implements ----------------

  virtual int custom_init() { return 0; }

  virtual int custom_deinit() { return 0; }

  virtual const std::string debug_string();

  // ------------------ OP Interface -------------------

  // Get the derived Channel object of current OP
  virtual Channel* mutable_channel() = 0;

  // Get the derived Channel object of current OP
  virtual const Channel* get_channel() const = 0;

  // Release the derived Channel object of current OP
  virtual int release_channel() = 0;

  // Inference interface
  virtual int inference() = 0;

  // ------------------ Conf Interface -------------------
  virtual void* create_config(const configure::DAGNode& conf) { return NULL; }

  virtual void delete_config(void* conf) {}

  virtual void set_config(void* conf) { return; }

  // ------------------ Metric Interface -------------------
  virtual void regist_metric() { return; }

 private:
  bool is_mutable(const std::string& op);

  bool is_mutable(const std::string& op) const;

  bool is_readable(const std::string& op);

  bool is_readable(const std::string& op) const;

 private:
  Bus* _bus;
  Dag* _dag;
  uint32_t _id;
  std::vector<std::string> _pre_node_names;  // for DAG execution
  std::string _name;
  std::string _full_name;  // service_workflow_stageindex_opname
  std::string _type;
  std::vector<std::string> _address;
  bool _has_calc;
  bool _has_init;
  TimerFlow* _timer;
  std::string _engine_name;  // only for infer_op
};

template <typename T>
class OpWithChannel : public Op {
 public:
  typedef T DataType;
  typedef OpChannel<T> ChannelType;

  OpWithChannel() : _channel(NULL) {}

  virtual ~OpWithChannel() {}

  // ---------- Implements ----------

  Channel* mutable_channel() {
    if (_channel != NULL) {
      return _channel;
    }

    // TODO(barriery): There are some problems in using butil::get_object
    // _channel = butil::get_object<ChannelType>();
    _channel = new ChannelType();

    if (!_channel) {
      LOG(ERROR) << "Failed mutable channel of type:" << typeid(T).name();
      return NULL;
    }
    _channel->init(this->id(), this->name());
    return _channel;
  }

  const Channel* get_channel() const { return _channel; }

  int release_channel() {
    if (_channel) {
      _channel->deinit();
      delete _channel;
    }
    // TODO(barriery): There are some problems in using butil::get_object
    /*
    if (_channel) {
      _channel->deinit();
      butil::return_object<ChannelType>(_channel);
    } */

    _channel = NULL;
    return 0;
  }

  // ------------- Interface -------------

  // Inference interface
  virtual int inference() = 0;

 private:
  ChannelType* _channel;
};

template <typename T, typename C>
class OpWithChannelAndConf : public OpWithChannel<T> {
 public:
  void set_config(void* conf) { _conf = static_cast<C*>(conf); }

  C* get_self_config() { return _conf; }

  virtual void delete_config(void* conf) { delete static_cast<C*>(conf); }

 private:
  C* _conf;
};

#define DECLARE_OP(OP_TYPE)           \
  OP_TYPE() { REGISTER_OP(OP_TYPE); } \
  static OP_TYPE _s_##OP_TYPE

#define DEFINE_OP(OP_TYPE) OP_TYPE OP_TYPE::_s_##OP_TYPE

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
