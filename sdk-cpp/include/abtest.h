/***************************************************************************
 * 
 * Copyright (c) 2018 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
/**
 * @file abtest.h
 * @author wanlijin01(wanlijin01@baidu.com)
 * @date 2018/07/06 17:11:38
 * @brief 
 *  
 **/

#ifndef  BAIDU_PADDLE_SERVING_SDK_CPP_ABTEST_H
#define  BAIDU_PADDLE_SERVING_SDK_CPP_ABTEST_H

#include "stub.h"
#include "common.h"
#include "factory.h"
#include <google/protobuf/message.h>

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

class Stub;
class Variant;

static const std::string WEIGHT_SEPERATOR = "|";

class EndpointRouterBase {
public:
    typedef std::vector<Variant*> VariantList;

    virtual ~EndpointRouterBase() {}

    virtual int initialize(
            const google::protobuf::Message& conf) = 0;

    virtual Variant* route(const VariantList&) = 0;

    virtual Variant* route(
            const VariantList&,
            const void*) = 0;
};

class WeightedRandomRender : public EndpointRouterBase {
public:
    static int register_self() {
        INLINE_REGIST_OBJECT(WeightedRandomRender, EndpointRouterBase, -1);
        return 0;
    }

    WeightedRandomRender() : _normalized_sum(0) {}

    ~WeightedRandomRender() {}

    int initialize(
            const google::protobuf::Message& conf);

    Variant* route(const VariantList&);

    Variant* route(
            const VariantList&,
            const void*);

private:
    std::vector<uint32_t> _variant_weight_list;
    uint32_t _normalized_sum;
};

} // sdk_cpp
} // paddle_serving
} // baidu

#endif  //BAIDU_PADDLE_SERVING_SDK_CPP_ABTEST_H

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
