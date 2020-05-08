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
#include <stdint.h>
#include <string>
#include <vector>
#include "core/sdk-cpp/include/common.h"

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

#define PARSE_CONF_ITEM(conf, item, name, fail)          \
  do {                                                   \
    if (conf.has_##name()) {                             \
      item.set(conf.name());                             \
    } else {                                             \
      VLOG(2) << "Not found key in configue: " << #name; \
    }                                                    \
  } while (0)

#define ASSIGN_CONF_ITEM(dest, src, fail)                       \
  do {                                                          \
    if (!src.init) {                                            \
      VLOG(2) << "Cannot assign an unintialized item: " << #src \
              << " to dest: " << #dest;                         \
      return fail;                                              \
    }                                                           \
    dest = src.value;                                           \
  } while (0)

template <typename T>
struct type_traits {
  static type_traits<T> tag;
};

template <typename T>
type_traits<T> type_traits<T>::tag;

template <typename T>
struct ConfigItem {
  T value;
  bool init;
  ConfigItem() : init(false) {}
  void set(const T& unit) {
    value = unit;
    init = true;
  }
};

struct Connection {
  ConfigItem<int32_t> tmo_conn;
  ConfigItem<int32_t> tmo_rpc;
  ConfigItem<int32_t> tmo_hedge;
  ConfigItem<uint32_t> cnt_retry_conn;
  ConfigItem<uint32_t> cnt_retry_hedge;
  ConfigItem<uint32_t> cnt_maxconn_per_host;
  ConfigItem<std::string> type_conn;
};

struct NamingInfo {
  ConfigItem<std::string> cluster_naming;
  ConfigItem<std::string> load_balancer;
  ConfigItem<std::string> cluster_filter;
};

struct RpcParameters {
  ConfigItem<std::string> protocol;
  ConfigItem<int32_t> compress_type;
  ConfigItem<uint32_t> package_size;
  ConfigItem<std::string> route_tag;
  ConfigItem<uint32_t> max_channel;
};

struct SplitParameters {
  ConfigItem<std::string> split_tag;
  ConfigItem<std::string> tag_cands_str;
  std::vector<std::string> tag_values;
};

struct VariantInfo {
  VariantInfo() {}
  Connection connection;
  NamingInfo naminginfo;
  RpcParameters parameters;
  SplitParameters splitinfo;
};

struct EndpointInfo {
  EndpointInfo() : ab_test(NULL) {}
  std::string endpoint_name;
  std::string stub_service;
  std::vector<VariantInfo> vars;
  void* ab_test;
};

}  // namespace sdk_cpp
}  // namespace paddle_serving
}  // namespace baidu
