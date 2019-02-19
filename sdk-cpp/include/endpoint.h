/***************************************************************************
 * 
 * Copyright (c) 2018 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
/**
 * @file include/resource.h
 * @author wanlijin(wanlijin01@baidu.com)
 * @date 2018/07/06 17:06:25
 * @brief 
 *  
 **/

#ifndef  BAIDU_PADDLE_SERVING_SDK_ENDPOINT_H
#define  BAIDU_PADDLE_SERVING_SDK_ENDPOINT_H

#include "common.h"
#include "endpoint_config.h"
#include "stub.h"
#include "variant.h"
#include "predictor.h"

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

class Endpoint {
friend class EndpointRouterBase;
public:

    virtual ~Endpoint() {}

    Endpoint() {
        _variant_list.clear();
    }

    int initialize(const EndpointInfo& ep_info);

    int thrd_initialize();

    int thrd_clear();

    int thrd_finalize();

    Predictor* get_predictor(const void* params);

    Predictor* get_predictor();

    int ret_predictor(Predictor* predictor);

    const std::string& endpoint_name() const {
        return _endpoint_name;
    }

private:
    int initialize_variant(
            const VariantInfo& var_info,
            const std::string& service,
            const std::string& ep_name,
            std::vector<Stub*>& stubs);

private:
    std::string       _endpoint_name;
    std::vector<Variant*> _variant_list;
};

} // sdk_cpp
} // paddle_serving
} // baidu

#endif  //BAIDU_PADDLE_SERVING_SDK_CPPRESOURCE_H

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
