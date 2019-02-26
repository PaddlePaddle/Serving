/***************************************************************************
 * 
 * Copyright (c) 2018 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file endpoint.cpp
 * @author wanlijin01(wanlijin01@baidu.com)
 * @date 2018/07/09 14:10:44
 * @brief 
 *  
 **/

#include "endpoint.h"
#include "factory.h"

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

int Endpoint::initialize(const EndpointInfo& ep_info) {
    _variant_list.clear();
    _endpoint_name = ep_info.endpoint_name;
    uint32_t var_size = ep_info.vars.size();
    for (uint32_t vi = 0; vi < var_size; ++vi) {
        const VariantInfo& var_info = ep_info.vars[vi];
        Variant* var = new (std::nothrow) Variant;
        if (!var || var->initialize(ep_info, var_info) != 0) {
            LOG(ERROR) << "Failed initialize variant, tag:" 
                << var_info.parameters.route_tag.value 
                << ", endpoint: " << ep_info.endpoint_name 
                << ", var index: " << vi;
            return -1;
        }
        _variant_list.push_back(var);
        LOG(INFO) << "Succ create variant: " << vi << ", endpoint:"
            << _endpoint_name;
    }

    return 0;
}

int Endpoint::thrd_initialize() {
    uint32_t var_size = _variant_list.size();
    for (uint32_t vi = 0; vi < var_size; ++vi) {
        Variant* var = _variant_list[vi];
        if (!var || var->thrd_initialize()) {
            LOG(ERROR) << "Failed thrd initialize var: " << vi;
            return -1;
        }
    }
    LOG(WARNING) << "Succ thrd initialize all vars: " << var_size;
    return 0;
}

int Endpoint::thrd_clear() {
    uint32_t var_size = _variant_list.size();
    for (uint32_t vi = 0; vi < var_size; ++vi) {
        Variant* var = _variant_list[vi];
        if (!var || var->thrd_clear()) {
            LOG(ERROR) << "Failed thrd clear var: " << vi;
            return -1;
        }
    }
    LOG(INFO) << "Succ thrd clear all vars: " << var_size;
    return 0;
}

int Endpoint::thrd_finalize() {
    uint32_t var_size = _variant_list.size();
    for (uint32_t vi = 0; vi < var_size; ++vi) {
        Variant* var = _variant_list[vi];
        if (!var || var->thrd_finalize()) {
            LOG(ERROR) << "Failed thrd finalize var: " << vi;
            return -1;
        }
    }
    LOG(INFO) << "Succ thrd finalize all vars: " << var_size;
    return 0;
}

// 带全流量分层实验路由信息
Predictor* Endpoint::get_predictor(
        const void* params) {
    Variant* var = NULL;
    if (_variant_list.size() == 1) {
        var = _variant_list[0];
    }

    if (!var) {
        LOG(ERROR) << "get null var from endpoint.";
        return NULL;
    }

    return var->get_predictor(params);
}

Predictor* Endpoint::get_predictor() {
#if 1
    LOG(INFO) << "Endpoint::get_predictor";
#endif
    if (_variant_list.size() == 1) {
        if (_variant_list[0] == NULL) {
            LOG(ERROR) << "Not valid variant info"; 
            return NULL;
        }
        return _variant_list[0]->get_predictor();
    }
    
    return NULL;
}

int Endpoint::ret_predictor(Predictor* predictor) {
    const Stub* stub = predictor->stub();
    if (!stub || stub->return_predictor(
                predictor) != 0) {
        LOG(ERROR) << "Failed return predictor to pool";
        return -1;
    }

    return 0;
}

} // sdk_cpp
} // paddle_serving
} // baidu

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
