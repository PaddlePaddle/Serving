/***************************************************************************
 *
 * Copyright (c) 2018 Baidu.com, Inc. All Rights Reserved
 *
 **************************************************************************/

/**
 * @file endpoint_config.cpp
 * @author wanlijin01(com@baidu.com)
 * @date 2018/07/09 15:30:09
 * @brief
 *
 **/
#include "abtest.h"
#include "config_manager.h"
#include <brpc/server.h>

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

int EndpointConfigManager::create(const char* path, const char* file) {
    _endpoint_config_path = path;
    _endpoint_config_file = file;

    if (load() != 0) {
        LOG(FATAL) << "Failed reload endpoint config";
        return -1;
    }

    return 0;
}

int EndpointConfigManager::load() {
    try {
        comcfg::Configure conf;
        if (conf.load(
                    _endpoint_config_path.c_str(),
                    _endpoint_config_file.c_str()) != 0) {
            LOG(FATAL)
                << "Failed initialize endpoint list"
                << ", config: " << _endpoint_config_path
                << "/" << _endpoint_config_file;
            return -1;
        }

        VariantInfo default_var;
        if (init_one_variant(conf["DefaultVariantInfo"],
                    default_var) != 0) {
            LOG(FATAL) << "Failed read default var conf";
            return -1;
        }

        uint32_t ep_size = conf["Predictor"].size();
        for (uint32_t ei = 0; ei < ep_size; ++ei) {
            EndpointInfo ep;
            if (init_one_endpoint(conf["Predictor"][ei], ep,
                        default_var) != 0) {
                LOG(FATAL) << "Failed read endpoint info at: "
                        << ei;
                return -1;
            }

            std::map<std::string, EndpointInfo>::iterator it;
            if (_ep_map.find(ep.endpoint_name) != _ep_map.end()) {
                LOG(FATAL) << "Cannot insert duplicated endpoint"
                    << ", ep name: " << ep.endpoint_name;
            }

            std::pair<std::map<
                std::string, EndpointInfo>::iterator, bool> r
                = _ep_map.insert(std::make_pair(ep.endpoint_name, ep));
            if (!r.second) {
                LOG(FATAL) << "Failed insert endpoint, name"
                    << ep.endpoint_name;
                return -1;
            }
        }

    } catch (bsl::Exception& e) {
        LOG(FATAL) << "Failed load configure, err: " << e.what();
        return -1;
    }
    LOG(INFO)
        << "Success reload endpoint config file, id: "
        << _current_endpointmap_id;
    return 0;
}

int EndpointConfigManager::init_one_endpoint(
        const comcfg::ConfigUnit& conf, EndpointInfo& ep,
        const VariantInfo& dft_var) {
    try {
        // name
        ep.endpoint_name = conf["name"].to_cstr();
        // stub
        ep.stub_service = conf["service_name"].to_cstr();
        // abtest
        ConfigItem<std::string> ep_router;
        PARSE_CONF_ITEM(conf, ep_router, "endpoint_router", -1);
        if (ep_router.init) {
            std::string endpoint_router_info
                = conf["endpoint_router"].to_cstr();
            EndpointRouterBase* router
                = EndpointRouterFactory::instance().generate_object(
                        ep_router.value);
            if (!router || router->initialize(
                        conf[endpoint_router_info.c_str()]) != 0) {
                LOG(FATAL) << "Failed fetch valid ab test strategy"
                    << ", name:" << endpoint_router_info;
                return -1;
            }
            ep.ab_test = router;
        }

        // varlist
        uint32_t var_size = conf["VariantInfo"].size();
        for (uint32_t vi = 0; vi < var_size; ++vi) {
            VariantInfo var;
            if (merge_variant(dft_var, conf["VariantInfo"][vi],
                        var) != 0) {
                LOG(FATAL) << "Failed merge variant info at: "
                    << vi;
                return -1;
            }

            ep.vars.push_back(var);
        }

        if (ep.vars.size() > 1 && ep.ab_test == NULL) {
            LOG(FATAL) << "EndpointRouter must be configured, when"
                << " #Variants > 1.";
            return -1;
        }

        LOG(INFO)
            << "Succ load one endpoint, name: " << ep.endpoint_name
            << ", count of variants: " << ep.vars.size() << ".";

    } catch (bsl::Exception& e) {
        LOG(FATAL) << "Exception acccurs when load endpoint conf"
            << ", message: " << e.what();
        return -1;
    }
    return 0;
}

int EndpointConfigManager::init_one_variant(
        const comcfg::ConfigUnit& conf, VariantInfo& var) {
    try {
    // Connect
    const comcfg::ConfigUnit& conn = conf["Connection"];

    PARSE_CONF_ITEM(conn, var.connection.tmo_conn,
            "ConnectTimeoutMilliSec", -1);
    PARSE_CONF_ITEM(conn, var.connection.tmo_rpc,
            "RpcTimeoutMilliSec", -1);
    PARSE_CONF_ITEM(conn, var.connection.tmo_hedge,
            "HedgeRequestTimeoutMilliSec", -1);
    PARSE_CONF_ITEM(conn, var.connection.cnt_retry_conn,
            "ConnectRetryCount", -1);
    PARSE_CONF_ITEM(conn, var.connection.cnt_retry_hedge,
            "HedgeFetchRetryCount", -1);
    PARSE_CONF_ITEM(conn, var.connection.cnt_maxconn_per_host,
            "MaxConnectionPerHost", -1);
    PARSE_CONF_ITEM(conn, var.connection.type_conn,
            "ConnectionType", -1);

    // Naming
    const comcfg::ConfigUnit& name = conf["NamingInfo"];

    PARSE_CONF_ITEM(name, var.naminginfo.cluster_naming,
            "Cluster", -1);
    PARSE_CONF_ITEM(name, var.naminginfo.load_balancer,
            "LoadBalanceStrategy", -1);
    PARSE_CONF_ITEM(name, var.naminginfo.cluster_filter,
            "ClusterFilterStrategy", -1);

    // Rpc
    const comcfg::ConfigUnit& params = conf["RpcParameter"];

    PARSE_CONF_ITEM(params, var.parameters.protocol,
            "Protocol", -1);
    PARSE_CONF_ITEM(params, var.parameters.compress_type,
            "CompressType", -1);
    PARSE_CONF_ITEM(params, var.parameters.package_size,
            "PackageSize", -1);
    PARSE_CONF_ITEM(params, var.parameters.max_channel,
            "MaxChannelPerRequest", -1);
    // Split
    const comcfg::ConfigUnit& splits = conf["SplitInfo"];

    PARSE_CONF_ITEM(splits, var.splitinfo.split_tag,
            "split_tag_name", -1);
    PARSE_CONF_ITEM(splits, var.splitinfo.tag_cands_str,
            "tag_candidates", -1);
    if (parse_tag_values(var.splitinfo) != 0) {
        LOG(FATAL) << "Failed parse tag_values:" <<
            var.splitinfo.tag_cands_str.value;
        return -1;
    }

    // tag
    PARSE_CONF_ITEM(conf, var.parameters.route_tag,
            "Tag", -1);

    // router
    ConfigItem<std::string> var_router;
    PARSE_CONF_ITEM(conf, var_router, "variant_router", -1);
    if (var_router.init) {
        VariantRouterBase* router
            = VariantRouterFactory::instance().generate_object(
                    var_router.value);
        if (!router || router->initialize(
                    conf[var_router.value.c_str()]) != 0) {
            LOG(FATAL) << "Failed fetch valid variant router"
                << ", name:" << var_router.value;
            return -1;
        }
        var.ab_test = router;
    }

    } catch (...) {
        LOG(FATAL) << "Failed load variant from configure unit";
        return -1;
    }

    return 0;
}

int EndpointConfigManager::merge_variant(
        const VariantInfo& default_var,
        const comcfg::ConfigUnit& conf,
        VariantInfo& merged_var) {
    merged_var = default_var;

    // VariantRouter cannot be merged!
    merged_var.ab_test = NULL;
    return init_one_variant(conf, merged_var);
}

int EndpointConfigManager::parse_tag_values(
        SplitParameters& split) {

    split.tag_values.clear();
    if (!split.split_tag.init || !split.tag_cands_str.init) {
        LOG(WARNING) << "split info not set, skip...";
        return 0;
    }

    static const char SPLIT_DELIM = ',';
    const std::string& tag_str = split.tag_cands_str.value;
    std::string::size_type start_pos = 0;
    std::string::size_type end_pos;

    do {
        end_pos = tag_str.find(SPLIT_DELIM, start_pos);
        std::string tag_value_str;
        if (end_pos == std::string::npos) {
            tag_value_str = tag_str.substr(start_pos);
        } else {
            tag_value_str = tag_str.substr(
                    start_pos, end_pos - start_pos);
            start_pos = end_pos + 1;
        }

        split.tag_values.push_back(tag_value_str);
    } while (end_pos != std::string::npos);

    return 0;
}

} // sdk_cpp
} // paddle_serving
} // baidu

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
