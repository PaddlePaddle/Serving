/***************************************************************************
 *
 * Copyright (c) 2018 Baidu.com, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file src/variant.cpp
 * @author wanlijin01(wanlijin01@baidu.com)
 * @date 2018/07/27 17:42:21
 * @brief
 *
 **/

#include "variant.h"
#include "factory.h"

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

int Variant::initialize(const EndpointInfo& ep_info,
        const VariantInfo& var_info) {

    _endpoint_name = ep_info.endpoint_name;
    _stub_service = ep_info.stub_service;

    _variant_tag = var_info.parameters.route_tag.value;
    _stub_map.clear();

    const SplitParameters& split_info = var_info.splitinfo;
    uint32_t tag_size = split_info.tag_values.size();
    for (uint32_t ti = 0; ti < tag_size; ++ti) { // split
        Stub* stub = StubFactory::instance().generate_object(
                _stub_service);
        const std::string& tag_value = split_info.tag_values[ti];
        if (!stub || stub->initialize(var_info, ep_info.endpoint_name,
                    &split_info.split_tag.value, &tag_value) != 0) {
            LOG(ERROR) << "Failed init stub from factory"
                << ", stub name: " << ep_info.stub_service
                << ", filter tag: " << tag_value;
            return -1;
        }

        // 判重
        std::map<std::string, Stub*>::iterator iter =
            _stub_map.find(tag_value);
        if (iter != _stub_map.end()) {
            LOG(ERROR) << "duplicated tag value: "
                << tag_value;
            return -1;
        }
        _stub_map[tag_value] = stub;
    }

    if (_stub_map.size() > 0) {
        LOG(INFO) << "Initialize variants from VariantInfo"
            << ", stubs count: " << _stub_map.size();
        return 0;
    }

    Stub* stub = StubFactory::instance().generate_object(
                ep_info.stub_service);
    if (!stub || stub->initialize(
                var_info, _endpoint_name, NULL, NULL) != 0) {
        LOG(ERROR) << "Failed init stub from factory"
            << ", stub name: " << ep_info.stub_service;
        return -1;
    }

    _default_stub = stub;
    LOG(INFO) << "Succ create default debug";
    return 0;
}

int Variant::thrd_initialize() {
    if (_stub_map.size() <= 0) {
        return _default_stub->thrd_initialize();
    }

    std::map<std::string, Stub*>::iterator iter;
    for (iter = _stub_map.begin(); iter != _stub_map.end(); ++iter) {
        Stub* stub = iter->second;
        if (!stub || stub->thrd_initialize() != 0) {
            LOG(ERROR) << "Failed thrd initialize stub: " << iter->first; 
            return -1;
        }
        LOG(INFO) << "Succ thrd initialize stub:" << iter->first;
    }

    LOG(WARNING) << "Succ thrd initialize all stubs";
    return 0;
}

int Variant::thrd_clear() {
    if (_stub_map.size() <= 0) {
        return _default_stub->thrd_clear();
    }

    std::map<std::string, Stub*>::iterator iter;
    for (iter = _stub_map.begin(); iter != _stub_map.end(); ++iter) {
        Stub* stub = iter->second;
        if (!stub || stub->thrd_clear() != 0) {
            LOG(ERROR) << "Failed thrd clear stub: " << iter->first; 
            return -1;
        }
    }
    return 0;
}

int Variant::thrd_finalize() {
    if (_stub_map.size() <= 0) {
        return _default_stub->thrd_finalize();
    }

    std::map<std::string, Stub*>::iterator iter;
    for (iter = _stub_map.begin(); iter != _stub_map.end(); ++iter) {
        Stub* stub = iter->second;
        if (!stub || stub->thrd_finalize() != 0) {
            LOG(ERROR) << "Failed thrd finalize stub: " << iter->first; 
            return -1;
        }
    }
    return 0;
}

Predictor* Variant::get_predictor() {
    if (_default_stub) {
        return _default_stub->fetch_predictor();
    }

    return NULL;
}

Predictor* Variant::get_predictor(
        const void* params) {

    if (_default_stub) {
        return _default_stub->fetch_predictor();
    }

    return NULL;
}

} // sdk_cpp
} // paddle_serving
} // baidu

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
