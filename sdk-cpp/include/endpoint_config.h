/***************************************************************************
 * 
 * Copyright (c) 2018 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file endpoint_config.h
 * @author wanlijin01(wanlijin01@baidu.com)
 * @date 2018/07/12 15:17:56
 * @brief 
 *  
 **/

#include "common.h"

#ifndef  BAIDU_PADDLE_SERVING_SDK_CPP_ENDPOINT_CONFIG_H
#define  BAIDU_PADDLE_SERVING_SDK_CPP_ENDPOINT_CONFIG_H

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

#define PARSE_CONF_ITEM(conf, item, name, fail)             \
    do {                                                    \
        try {                                               \
            item.set(conf[name]);                           \
        } catch (comcfg::NoSuchKeyException& e) {           \
            LOG(INFO) << "Not found key in configue: " << name;\
        } catch (comcfg::ConfigException& e) {              \
            LOG(FATAL) << "Error config, key: " << name;    \
            return fail;                                    \
        } catch (...) {                                     \
            LOG(FATAL) << "Unkown error accurs when load config";\
            return fail;                                    \
        }                                                   \
    } while (0)

#define ASSIGN_CONF_ITEM(dest, src, fail)                           \
    do {                                                            \
        if (!src.init) {                                            \
            LOG(FATAL) << "Cannot assign an unintialized item: "    \
                << #src << " to dest: " << #dest;                   \
            return fail;                                            \
        }                                                           \
        dest = src.value;                                           \
    } while (0)

template<typename T> struct type_traits {
    static type_traits<T> tag;
};

template<typename T> struct ConfigItem {
    T value;
    bool init;
    ConfigItem() : init(false) {}
    void set(const comcfg::ConfigUnit& unit) {
        set_impl(type_traits<T>::tag, unit);
        init = true;
    }

    void set_impl(type_traits<int16_t>&,
            const comcfg::ConfigUnit& unit) {
        value = unit.to_int16();
    }

    void set_impl(type_traits<int32_t>&, 
            const comcfg::ConfigUnit& unit) {
        value = unit.to_int32();
    }

    void set_impl(type_traits<int64_t>&,
            const comcfg::ConfigUnit& unit) {
        value = unit.to_int64();
    }

    void set_impl(type_traits<uint16_t>&,
            const comcfg::ConfigUnit& unit) {
        value = unit.to_uint16();
    }

    void set_impl(type_traits<uint32_t>&,
            const comcfg::ConfigUnit& unit) {
        value = unit.to_uint32();
    }

    void set_impl(type_traits<uint64_t>&,
            const comcfg::ConfigUnit& unit) {
        value = unit.to_uint64();
    }

    void set_impl(type_traits<float>&,
            const comcfg::ConfigUnit& unit) {
        value = unit.to_float();
    }

    void set_impl(type_traits<double>&,
            const comcfg::ConfigUnit& unit) {
        value = unit.to_double();
    }

    void set_impl(type_traits<std::string>&,
            const comcfg::ConfigUnit& unit) {
        value = unit.to_cstr();
    }
};

struct Connection {
    ConfigItem<int32_t> tmo_conn;
    ConfigItem<int32_t> tmo_rpc;
    ConfigItem<int32_t> tmo_hedge;
    ConfigItem<uint32_t> cnt_retry_conn;
    ConfigItem<uint32_t> cnt_retry_hedge;
    ConfigItem<uint32_t> cnt_maxconn_per_host;
    ConfigItem<std::string> type_conn;
};

struct NamingInfo {
    ConfigItem<std::string> cluster_naming;
    ConfigItem<std::string> load_balancer;
    ConfigItem<std::string> cluster_filter;
};

struct RpcParameters {
    ConfigItem<std::string> protocol;
    ConfigItem<int32_t> compress_type;
    ConfigItem<uint32_t> package_size;
    ConfigItem<std::string> route_tag;
    ConfigItem<uint32_t> max_channel;
};

struct SplitParameters {
    ConfigItem<std::string> split_tag;
    ConfigItem<std::string> tag_cands_str;
    std::vector<std::string> tag_values;
};

struct VariantInfo {
    VariantInfo() : ab_test(NULL) {}
    Connection connection;
    NamingInfo naminginfo;
    RpcParameters parameters;
    SplitParameters splitinfo;
    void* ab_test;
};

struct EndpointInfo {
    EndpointInfo() : ab_test(NULL) {}
    std::string endpoint_name;
    std::string stub_service;
    std::vector<VariantInfo> vars;
    void* ab_test;
};

} // sdk_cpp
} // paddle_serving
} // baidu

#endif  //BAIDU_PADDLE_SERVING_SDK_CPP_ENDPOINT_CONFIG_H

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
