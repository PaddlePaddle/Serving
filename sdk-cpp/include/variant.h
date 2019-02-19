/***************************************************************************
 * 
 * Copyright (c) 2018 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file include/variant.h
 * @author wanlijin01(wanlijin01@baidu.com)
 * @date 2018/07/27 17:37:31
 * @brief 
 *  
 **/

#ifndef BAIDU_PADDLE_SERVING_CPP_SDK_VARIANT_H
#define BAIDU_PADDLE_SERVING_CPP_SDK_VARIANT_H

#include "common.h"
#include "endpoint_config.h"
#include "stub.h"
#include "predictor.h"

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

class Variant {
friend class VariantRouterBase;
public:

    virtual ~Variant() {}

    Variant() : _default_stub(NULL) {
        _stub_map.clear();
    }

    int initialize(
            const EndpointInfo& ep_info,
            const VariantInfo& var_info);

    int thrd_initialize();

    int thrd_clear();

    int thrd_finalize();

    Predictor* get_predictor(
            const void* params);

    Predictor* get_predictor();

    int ret_predictor(Predictor* predictor);

    const std::string& variant_tag() const {
        return _variant_tag;
    }

private:
    std::string _endpoint_name;
    std::string _stub_service;

    std::string _variant_tag;
    std::map<std::string, Stub*> _stub_map;
    Stub* _default_stub;
};

} // sdk_cpp
} // paddle_serving
} // baidu

#endif  //BAIDU_PADDLE_SERVING_CPP_SDK_VARIANT_H

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
