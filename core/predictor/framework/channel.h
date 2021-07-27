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
#include <utility>
#include "core/predictor/common/inner_common.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

class Channel;

class Bus {
 public:
  Bus() { clear(); }

  int regist(const std::string& op, Channel* channel) {
    std::pair<boost::unordered_map<std::string, Channel*>::iterator, bool> r =
        _op_channels.insert(std::make_pair(op, channel));
    if (!r.second) {
      LOG(ERROR) << "Failed insert op&channel into bus:" << op;
      return -1;
    }
    return 0;
  }

  Channel* channel_by_name(const std::string& op_name) {
    typename boost::unordered_map<std::string, Channel*>::iterator it =
        _op_channels.find(op_name);
    if (it == _op_channels.end()) {
      LOG(WARNING) << "Not found channel in bus, op_name:" << op_name << ".";
      return NULL;
    }

    return it->second;
  }

  void clear() { _op_channels.clear(); }

  size_t size() const { return _op_channels.size(); }

 private:
  boost::unordered_map<std::string, Channel*> _op_channels;
};

class Channel {
 public:
  Channel() {}

  void init(uint32_t id, const char* op) {
    _id = id;
    _op = std::string(op);
    clear_data();
  }

  void deinit() { clear_data(); }

  uint32_t id() const { return _id; }

  const std::string& op() { return _op; }

  int share_to_bus(Bus* bus, const uint64_t log_id) {
    if (bus->regist(_op, this) != 0) {
      LOG(ERROR) << "(logid=" << log_id << ") Failed regist channel[" << _op
                 << "] to bus!";
      return -1;
    }

    return 0;
  }

  virtual void clear_data() = 0;

  virtual void* param() = 0;
  virtual const void* param() const = 0;

  virtual google::protobuf::Message* message() = 0;
  virtual const google::protobuf::Message* message() const = 0;

  virtual Channel& operator=(const Channel& channel) = 0;

  virtual std::string debug_string() const = 0;

 private:
  uint32_t _id;
  std::string _op;
};

template <typename T>
class OpChannel : public Channel {
 public:
  OpChannel() {}

  void clear_data() { _data.Clear(); }

  void* param() { return &_data; }

  const void* param() const { return &_data; }

  google::protobuf::Message* message() {
    return message_impl(
        derived_from_message<
            TIsDerivedFromB<T, google::protobuf::Message>::RESULT>());
  }

  google::protobuf::Message* message_impl(derived_from_message<true>) {
    return dynamic_cast<google::protobuf::Message*>(&_data);
  }

  google::protobuf::Message* message_impl(derived_from_message<false>) {
    LOG(ERROR) << "Current type: " << typeid(T).name()
               << " is not derived from protobuf.";
    return NULL;
  }

  const google::protobuf::Message* message() const {
    return message_impl(
        derived_from_message<
            TIsDerivedFromB<T, google::protobuf::Message>::RESULT>());
  }

  const google::protobuf::Message* message_impl(
      derived_from_message<true>) const {
    return dynamic_cast<const google::protobuf::Message*>(&_data);
  }

  const google::protobuf::Message* message_impl(
      derived_from_message<false>) const {
    LOG(ERROR) << "Current type: " << typeid(T).name()
               << " is not derived from protobuf.";
    return NULL;
  }

  Channel& operator=(const Channel& channel) {
    _data = *(dynamic_cast<const OpChannel<T>&>(channel)).data();
    return *this;
  }

  std::string debug_string() const { return _data.ShortDebugString(); }

  // functions of derived class

  T* data() { return &_data; }

  const T* data() const { return &_data; }

  Channel& operator=(const T& obj) {
    _data = obj;
    return *this;
  }

 private:
  T _data;
};

template <>
class OpChannel<google::protobuf::Message> : public Channel {
 public:
  OpChannel<google::protobuf::Message>() : _data(NULL) {}

  virtual ~OpChannel<google::protobuf::Message>() { _data = NULL; }

  void clear_data() { _data = NULL; }

  void* param() { return const_cast<void*>((const void*)_data); }

  const void* param() const { return _data; }

  google::protobuf::Message* message() {
    return const_cast<google::protobuf::Message*>(_data);
  }

  const google::protobuf::Message* message() const { return _data; }

  Channel& operator=(const Channel& channel) {
    _data = channel.message();
    return *this;
  }

  std::string debug_string() const {
    if (_data) {
      return _data->ShortDebugString();
    } else {
      return "{\"Error\": \"Null Message Ptr\"}";
    }
  }

  // derived function imiplements
  google::protobuf::Message* data() {
    return const_cast<google::protobuf::Message*>(_data);
  }

  const google::protobuf::Message* data() const { return _data; }

  OpChannel<google::protobuf::Message>& operator=(
      google::protobuf::Message* message) {
    _data = message;
    return *this;
  }

  OpChannel<google::protobuf::Message>& operator=(
      const google::protobuf::Message* message) {
    _data = message;
    return *this;
  }

 private:
  const google::protobuf::Message* _data;
};

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
