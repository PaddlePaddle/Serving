#include <brpc/policy/nova_pbrpc_protocol.h> // NovaServiceAdaptor
#include <brpc/policy/public_pbrpc_protocol.h> // PublicPbrpcServiceAdaptor
#include <brpc/policy/nshead_mcpack_protocol.h> // NsheadMcpackAdaptor
#include "common/inner_common.h"
#include "framework/server.h"
#include "framework/service_manager.h"
#include "framework/resource.h"
#include "framework/manager.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

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
    if (FLAGS_enable_nshead_protocol) {
        LOG(INFO) << "FLAGS_enable_nshead_protocol on, try to set FLAGS_nshead_protocol["
                << FLAGS_nshead_protocol << "] in server side";
        _set_server_option_by_protocol(FLAGS_nshead_protocol);
    }
    _options.max_concurrency = FLAGS_max_concurrency;
    _options.num_threads = FLAGS_num_threads;
}

int ServerManager::add_service_by_format(const std::string& format) {
  Service* service = 
      FormatServiceManager::instance().get_service(format);
  if (service == NULL) {
    LOG(FATAL) << "Not found service by format:" << format << "!";
    return -1;
  }

  if (_format_services.find(format) != _format_services.end()) {
    LOG(FATAL) << "Cannot insert duplicated service by format:" 
        << format << "!";
    return -1;
  }

  std::pair<boost::unordered_map<std::string, Service*>::iterator, bool> it 
      = _format_services.insert(std::make_pair(format, service));
  if (!it.second) {
    LOG(FATAL) << "Failed insert service by format:"
        << format << "!";
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
  for (it = _format_services.begin(); it != _format_services.end();
          it++) {
    if (_server.AddService(it->second, brpc::SERVER_DOESNT_OWN_SERVICE)
            != 0) {
      LOG(ERROR) << "Failed to add service of format:"
          << it->first << "!";
      return -1;
    }
  }

  if (_server.Start(FLAGS_port, &_options) != 0) {
    LOG(ERROR) << "Failed to start Paddle Inference Server" ;
    return -1;
  }
  _server.RunUntilAskedToQuit();

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
        _options.nshead_service = new ::brpc::policy::NovaServiceAdaptor;;
    } else if (_compare_string_piece_without_case(protocol_type, "public_pbrpc")) {
        _options.nshead_service = new ::brpc::policy::PublicPbrpcServiceAdaptor;
    } else if (_compare_string_piece_without_case(protocol_type, "nshead_mcpack")) {
        _options.nshead_service = new ::brpc::policy::NsheadMcpackAdaptor;
    } else {
        LOG(ERROR) << "fail to set nshead protocol, protocol_type[" << protocol_type << "].";
        return;
    }
    _options.enabled_protocols = enabled_protocols;
    LOG(INFO) << "success to set nshead protocol, protocol_type[" << protocol_type << "].";
}

int ServerManager::_start_reloader() {
    int ret = THREAD_CREATE(
            &_reload_thread, NULL,
            ServerManager::_reload_worker,
            NULL);

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
            LOG(FATAL) << "Failed reload resource!";
        }   

        if (WorkflowManager::instance().reload() != 0) {
            LOG(FATAL) << "Failed reload workflows"; 
        }

        usleep(FLAGS_reload_interval_s * 1000000);
    }

    LOG(INFO) << "Exit reload worker!";
    return NULL;
}

} // predictor
} // paddle_serving
} // baidu
