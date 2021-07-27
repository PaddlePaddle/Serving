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
#include "core/predictor/common/inner_common.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

#ifdef BCLOUD
namespace brpc = baidu::rpc;
#endif

class ServerManager {
 public:
  typedef google::protobuf::Service Service;
  ServerManager();

  static ServerManager& instance() {
    static ServerManager server;
    return server;
  }

  static bool reload_starting() { return _s_reload_starting; }

  static void stop_reloader() { _s_reload_starting = false; }

  int add_service_by_format(const std::string& format);

  int start_and_wait();

 private:
  int _start_reloader();

  int _wait_reloader();

  static void* _reload_worker(void* args);

  bool _compare_string_piece_without_case(const butil::StringPiece& s1,
                                          const char* s2);

  void _set_server_option_by_protocol(
      const ::butil::StringPiece& protocol_type);

  brpc::ServerOptions _options;
  brpc::Server _server;
  boost::unordered_map<std::string, Service*> _format_services;
  THREAD_T _reload_thread;
  static volatile bool _s_reload_starting;
};

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
