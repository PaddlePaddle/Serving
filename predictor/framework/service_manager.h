#ifndef BAIDU_PADDLE_SERVING_PREDICTOR_FORMAT_MANAGER_H
#define BAIDU_PADDLE_SERVING_PREDICTOR_FORMAT_MANAGER_H

#include "common/inner_common.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

#define REGIST_FORMAT_SERVICE(svr_name, svr)                    \
do {                                                            \
    int ret = ::baidu::paddle_serving::predictor::FormatServiceManager::instance().regist_service(\
              svr_name, svr);                                   \
    if (ret != 0) {                                             \
        LOG(ERROR)                                              \
            << "Failed regist service["                         \
            << svr_name << "]" << "["                           \
            << typeid(svr).name() << "]"                        \
            << "!";                                             \
    } else {                                                    \
        LOG(INFO)                                               \
            << "Success regist service["                        \
            << svr_name << "]["                                 \
            << typeid(svr).name() << "]"                        \
            << "!";                                             \
    }                                                           \
} while (0)

class FormatServiceManager {
public:
    typedef google::protobuf::Service Service;

    int regist_service(const std::string& svr_name, Service* svr) {
        if (_service_map.find(svr_name) != _service_map.end()) {
            LOG(ERROR) 
                << "Service[" << svr_name << "]["
                << typeid(svr).name() << "]"
                << " already exist!";
            return -1;
        }

        std::pair<boost::unordered_map<std::string, Service*>::iterator, bool> ret;
        ret = _service_map.insert(std::make_pair(svr_name, svr));
        if (ret.second == false) {
            LOG(ERROR)
                << "Service[" << svr_name << "]["
                << typeid(svr).name() << "]"
                << " insert failed!";
            return -1;
        }

        LOG(INFO)
            << "Service[" << svr_name << "] insert successfully!";
        return 0;
    }

    Service* get_service(const std::string& svr_name) {
        boost::unordered_map<std::string, Service*>::iterator res;
        if ((res = _service_map.find(svr_name)) == _service_map.end()) {
            LOG(WARNING) 
                << "Service[" << svr_name << "] "
                << "not found in service manager"
                << "!";
            return NULL;
        }
        return (*res).second;
    }

    static FormatServiceManager& instance() {
        static FormatServiceManager service_;
        return service_;
    }

private:
    boost::unordered_map<std::string, Service*> _service_map;
};

} // predictor
} // paddle_serving
} // baidu

#endif
