/***************************************************************************
 * 
 * Copyright (c) 2018 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
/**
 * @file include/stub.h
 * @author wanlijin(wanlijin01@baidu.com)
 * @date 2018/12/04 16:42:29
 * @brief 
 *  
 **/

#ifndef BAIDU_PADDLE_SERVING_SDK_CPP_STUB_H
#define BAIDU_PADDLE_SERVING_SDK_CPP_STUB_H

#include "common.h"

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

class Predictor;
struct VariantInfo;

class Stub {
public:
    typedef google::protobuf::Message Message;

    virtual ~Stub() {} 

    virtual int initialize(const VariantInfo& var, const std::string& ep,
            const std::string* tag, const std::string* tag_value) = 0;

    // predictor
    virtual Predictor* fetch_predictor() = 0;
    virtual int return_predictor(Predictor* predictor) = 0;
    virtual int return_predictor(Predictor* predictor) const = 0;

    // request
    virtual Message* fetch_request() = 0;
    virtual int return_request(Message* request) = 0;
    virtual int return_request(Message* request) const = 0;

    // response
    virtual Message* fetch_response() = 0;
    virtual int return_response(Message* response) = 0;
    virtual int return_response(Message* response) const = 0;

    virtual const std::string& which_endpoint() const = 0;

    // control logic for tls
    virtual int thrd_initialize() = 0;

    virtual int thrd_clear() = 0;

    virtual int thrd_finalize() = 0;

    virtual void update_average(int64_t acc, const char* name) = 0;
    virtual void update_latency(int64_t acc, const char* name) = 0;
};

} // sdk_cpp
} // paddle_serving
} // baidu

#endif  //BAIDU_PADDLE_SERVING_SDK_CPP_STUB_H

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
