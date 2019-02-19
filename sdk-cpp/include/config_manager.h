/***************************************************************************
 * 
 * Copyright (c) 2018 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file config_manager.h
 * @author wanlijin01(wanlijin01@baidu.com)
 * @date 2018/07/09 15:28:43
 * @brief 
 *  
 **/

#ifndef  BAIDU_PADDLE_SERVING_SDK_CPP_CONFIG_MANAGER_H
#define  BAIDU_PADDLE_SERVING_SDK_CPP_CONFIG_MANAGER_H

#include "common.h"
#include "endpoint_config.h"

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

class EndpointConfigManager {
public:
    static EndpointConfigManager& instance() {
        static EndpointConfigManager singleton;
        return singleton;
    }

    EndpointConfigManager() 
        : _last_update_timestamp(0),
        _current_endpointmap_id(1) {}

    int create(const char* path, const char* file);

    int load();

    bool need_reload() {
        return false;
    }

    int reload() {
        if (!need_reload()) {
            LOG(INFO) << "Noneed reload endpoin config";
            return 0;
        }

        return load();
    }

    const std::map<std::string, EndpointInfo>& config() {
        return _ep_map;
    }

    const std::map<std::string, EndpointInfo>& config() const {
        return _ep_map;
    }

private:
    int init_one_variant(
            const comcfg::ConfigUnit& conf, 
            VariantInfo& var);

    int init_one_endpoint(
            const comcfg::ConfigUnit& conf,
            EndpointInfo& ep,
            const VariantInfo& default_var);

    int merge_variant(
            const VariantInfo& default_var,
            const comcfg::ConfigUnit& conf,
            VariantInfo& merged_var);

    int parse_tag_values(
            SplitParameters& split);
    
private:
    std::map<std::string, EndpointInfo> _ep_map;
    std::string _endpoint_config_path;
    std::string _endpoint_config_file;
    uint32_t _last_update_timestamp;
    uint32_t _current_endpointmap_id;
};

} // sdk_cpp
} // paddle_serving
} // baidu

#endif  //BAIDU_PADDLE_SERVING_SDK_CPP_CONFIG_MANAGER_H

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
