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

#include "core/predictor/framework/server.h"
#ifdef BCLOUD
#include <baidu/rpc/policy/nova_pbrpc_protocol.h>     // NovaServiceAdaptor
#include <baidu/rpc/policy/nshead_mcpack_protocol.h>  // NsheadMcpackAdaptor
#include <baidu/rpc/policy/public_pbrpc_protocol.h>  // PublicPbrpcServiceAdaptor
#else
#include <brpc/policy/nova_pbrpc_protocol.h>     // NovaServiceAdaptor
#include <brpc/policy/nshead_mcpack_protocol.h>  // NsheadMcpackAdaptor
#include <brpc/policy/public_pbrpc_protocol.h>   // PublicPbrpcServiceAdaptor
#endif
#include <string>
#include <utility>
#include "core/predictor/common/inner_common.h"
#include "core/predictor/framework/manager.h"
#include "core/predictor/framework/resource.h"
#include "core/predictor/framework/service_manager.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

#ifdef BCLOUD
namespace brpc = baidu::rpc;
#endif

volatile bool ServerManager::_s_reload_starting = true;

bool ServerManager::_compare_string_piece_without_case(
    const butil::StringPiece& s1, const char* s2) {
  if (strlen(s2) != s1.size()) {
    return false;
  }
  return strncasecmp(s1.data(), s2, s1.size()) == 0;
}

ServerManager::ServerManager() {
  _format_services.clear();
  _options.idle_timeout_sec = FLAGS_idle_timeout_s;
  _options.max_concurrency = FLAGS_max_concurrency;
  _options.num_threads = FLAGS_num_threads;
}

int ServerManager::add_service_by_format(const std::string& format) {
  Service* service = FormatServiceManager::instance().get_service(format);
  if (service == NULL) {
    LOG(ERROR) << "Not found service by format:" << format << "!";
    return -1;
  }

  if (_format_services.find(format) != _format_services.end()) {
    LOG(ERROR) << "Cannot insert duplicated service by format:" << format
               << "!";
    return -1;
  }

  std::pair<boost::unordered_map<std::string, Service*>::iterator, bool> it =
      _format_services.insert(std::make_pair(format, service));
  if (!it.second) {
    LOG(ERROR) << "Failed insert service by format:" << format << "!";
    return -1;
  }

  return 0;
}

int ServerManager::start_and_wait() {
  if (_start_reloader() != 0) {
    LOG(ERROR) << "Failed start reloader";
    return -1;
  }

  boost::unordered_map<std::string, Service*>::iterator it;
  for (it = _format_services.begin(); it != _format_services.end(); it++) {
    if (_server.AddService(it->second, brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
      LOG(ERROR) << "Failed to add service of format:" << it->first << "!";
      return -1;
    }
  }

  // rpc multi-thread start from here.
  if (_server.Start(FLAGS_port, &_options) != 0) {
    LOG(ERROR) << "Failed to start Paddle Inference Server";
    return -1;
  }

  std::cout << "C++ Serving service started successfully!" << std::endl;
  LOG(INFO) << "C++ Serving service started successfully!";

  _server.RunUntilAskedToQuit();

  ServerManager::stop_reloader();
  if (_wait_reloader() != 0) {
    LOG(ERROR) << "Failed start reloader";
    return -1;
  }
  return 0;
}

void ServerManager::_set_server_option_by_protocol(
    const ::butil::StringPiece& protocol_type) {
  std::string enabled_protocols = FLAGS_enable_protocol_list;
  if (_compare_string_piece_without_case(protocol_type, "nova_pbrpc")) {
    _options.nshead_service = new ::brpc::policy::NovaServiceAdaptor;
  } else if (_compare_string_piece_without_case(protocol_type,
                                                "public_pbrpc")) {
    _options.nshead_service = new ::brpc::policy::PublicPbrpcServiceAdaptor;
  } else if (_compare_string_piece_without_case(protocol_type,
                                                "nshead_mcpack")) {
    _options.nshead_service = new ::brpc::policy::NsheadMcpackAdaptor;
  } else {
    LOG(ERROR) << "fail to set nshead protocol, protocol_type[" << protocol_type
               << "].";
    return;
  }
  _options.enabled_protocols = enabled_protocols;
  LOG(INFO) << "success to set nshead protocol, protocol_type[" << protocol_type
            << "].";
}

int ServerManager::_start_reloader() {
  int ret =
      THREAD_CREATE(&_reload_thread, NULL, ServerManager::_reload_worker, NULL);

  if (ret != 0) {
    LOG(ERROR) << "Failed start reload thread, ret:" << ret;
    return -1;
  }

  return 0;
}

int ServerManager::_wait_reloader() {
  THREAD_JOIN(_reload_thread, NULL);
  return 0;
}

void* ServerManager::_reload_worker(void* args) {
  LOG(INFO) << "Entrence reload worker, "
            << "interval_s: " << FLAGS_reload_interval_s;
  while (ServerManager::reload_starting()) {
    LOG(INFO) << "Begin reload framework...";
    if (Resource::instance().reload() != 0) {
      LOG(ERROR) << "Failed reload resource!";
    }

    if (WorkflowManager::instance().reload() != 0) {
      LOG(ERROR) << "Failed reload workflows";
    }

    usleep(FLAGS_reload_interval_s * 1000000);
  }

  LOG(INFO) << "Exit reload worker!";
  return NULL;
}

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
