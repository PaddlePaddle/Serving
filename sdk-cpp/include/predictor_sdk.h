/***************************************************************************
 * 
 * Copyright (c) 2018 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
/**
 * @file include/predictor_api.h
 * @author wanlijin01(wanlijin01@baidu.com)
 * @date 2018/07/09 17:33:59
 * @brief 
 *  
 **/

#ifndef  BAIDU_PADDLE_SERVING_SDK_CPP_PREDICTOR_SDK_H
#define  BAIDU_PADDLE_SERVING_SDK_CPP_PREDICTOR_SDK_H

#include "stub.h"
#include "predictor.h"
#include "endpoint_config.h"
#include "endpoint.h"
#include "config_manager.h"

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

class PredictorApi {
public:
    PredictorApi() {}

    int register_all();

    int create(const char* path, const char* file);

    int thrd_initialize();

    int thrd_clear();

    int thrd_finalize();

    void destroy();

    static PredictorApi& instance() {
        static PredictorApi api;
        return api;
    }

    Predictor* fetch_predictor(std::string ep_name) {
        std::map<std::string, Endpoint*>::iterator it
            = _endpoints.find(ep_name);
        if (it == _endpoints.end() || !it->second) {
            LOG(ERROR) << "Failed fetch predictor:"
                << ", ep_name: " << ep_name;
            return NULL;
        }
        return it->second->get_predictor();
    }

    Predictor* fetch_predictor(std::string ep_name,
            const void* params) {
        std::map<std::string, Endpoint*>::iterator it
            = _endpoints.find(ep_name);
        if (it == _endpoints.end() || !it->second) {
            LOG(ERROR) << "Failed fetch predictor:"
                << ", ep_name: " << ep_name;
            return NULL;
        }
        return it->second->get_predictor(params);
    }

    int free_predictor(Predictor* predictor) {
        const Stub* stub = predictor->stub();
        if (!stub || stub->return_predictor(predictor) != 0) {
            LOG(ERROR) << "Failed return predictor via stub";
            return -1;
        }

        return 0;
    }

private:
    EndpointConfigManager _config_manager;
    std::map<std::string, Endpoint*> _endpoints;
};

} // sdk_cpp
} // paddle_serving
} // baidu

#endif  //BAIDU_PADDLE_SERVING_SDK_CPP_PREDICTOR_SDK_H

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
