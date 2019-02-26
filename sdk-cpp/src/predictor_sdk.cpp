/***************************************************************************
 * 
 * Copyright (c) 2018 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
/**
 * @file src/predictor_api.cpp
 * @author wanlijin01(wanlijin01@baidu.com)
 * @date 2018/07/09 17:36:13
 * @brief 
 *  
 **/
#include "abtest.h"
#include "predictor_sdk.h"

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

int PredictorApi::register_all() {
    if (WeightedRandomRender::register_self() != 0) {
        LOG(ERROR) << "Failed register WeightedRandomRender";
        return -1;
    }

    LOG(WARNING) << "Succ register all components!";

    return 0;
}

int PredictorApi::create(const char* path, const char* file) {
    if (register_all() != 0) {
        LOG(ERROR)  << "Failed do register all!";
        return -1;
    }

    if (_config_manager.create(path, file) != 0) {
        LOG(ERROR) << "Failed create config manager from conf:" 
            << path << "/" << file;
        return -1;
    }

    const std::map<std::string, EndpointInfo>& map 
        = _config_manager.config();
    std::map<std::string, EndpointInfo>::const_iterator it;
    for (it = map.begin(); it != map.end(); ++it) {
        const EndpointInfo& ep_info = it->second;
        Endpoint* ep = new (std::nothrow) Endpoint();
        if (ep->initialize(ep_info) != 0) {
            LOG(ERROR) << "Failed intialize endpoint:"
                << ep_info.endpoint_name;
            return -1;
        }

        if (_endpoints.find(
                    ep_info.endpoint_name) != _endpoints.end()) {
            LOG(ERROR) << "Cannot insert duplicated endpoint:"
                << ep_info.endpoint_name;
            return -1;
        }

        std::pair<std::map<std::string, Endpoint*>::iterator, bool> r
            = _endpoints.insert(std::make_pair(
                        ep_info.endpoint_name, ep));
        if (!r.second) {
            LOG(ERROR) << "Failed insert endpoint:" 
                << ep_info.endpoint_name;
            return -1;
        }

        LOG(INFO) << "Succ create endpoint instance with name: "
            << ep_info.endpoint_name;
    }

    return 0;
}

int PredictorApi::thrd_initialize() {
    std::map<std::string, Endpoint*>::const_iterator it;
    for (it = _endpoints.begin(); it != _endpoints.end(); ++it) {
        Endpoint* ep = it->second;
        if (ep->thrd_initialize() != 0) {
            LOG(ERROR) << "Failed thrd initialize endpoint:"
                << it->first;
            return -1;
        }

        LOG(WARNING) << "Succ thrd initialize endpoint:"
            << it->first;
    }
    return 0;
}

int PredictorApi::thrd_clear() {
    std::map<std::string, Endpoint*>::const_iterator it;
    for (it = _endpoints.begin(); it != _endpoints.end(); ++it) {
        Endpoint* ep = it->second;
        if (ep->thrd_clear() != 0) {
            LOG(ERROR) << "Failed thrd clear endpoint:"
                << it->first;
            return -1;
        }

        LOG(INFO) << "Succ thrd clear endpoint:"
            << it->first;
    }
    return 0;
}

int PredictorApi::thrd_finalize() {
    std::map<std::string, Endpoint*>::const_iterator it;
    for (it = _endpoints.begin(); it != _endpoints.end(); ++it) {
        Endpoint* ep = it->second;
        if (ep->thrd_finalize() != 0) {
            LOG(ERROR) << "Failed thrd finalize endpoint:"
                << it->first;
            return -1;
        }

        LOG(INFO) << "Succ thrd finalize endpoint:"
            << it->first;
    }
    return 0;
}

void PredictorApi::destroy() {
    // TODO
    return ;
}

} // sdk_cpp
} // paddle_serving
} // baidu

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
