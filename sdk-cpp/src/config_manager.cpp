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

using configure::SDKConf;

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
        SDKConf sdk_conf;
        if (configure::read_proto_conf(
                    _endpoint_config_path.c_str(),
                    _endpoint_config_file.c_str(),
                    &sdk_conf) != 0) {
            LOG(FATAL)
                << "Failed initialize endpoint list"
                << ", config: " << _endpoint_config_path
                << "/" << _endpoint_config_file;
            return -1;
        }

        VariantInfo default_var;
        if (init_one_variant(sdk_conf.default_variant_conf(),
                    default_var) != 0) {
            LOG(FATAL) << "Failed read default var conf";
            return -1;
        }

        uint32_t ep_size = sdk_conf.predictors_size();
        for (uint32_t ei = 0; ei < ep_size; ++ei) {
            EndpointInfo ep;
            if (init_one_endpoint(sdk_conf.predictors(ei), ep,
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
        const configure::Predictor& conf, EndpointInfo& ep,
        const VariantInfo& dft_var) {
    try {
        // name
        ep.endpoint_name = conf.name();
        // stub
        ep.stub_service = conf.service_name();
        // abtest
        ConfigItem<std::string> ep_router;
        PARSE_CONF_ITEM(conf, ep_router, endpoint_router, -1);
        if (ep_router.init) {
            if (ep_router.value != "WeightedRandomRenderConf") {
                LOG(FATAL) << "endpointer_router unrecognized " << ep_router.value;
                return -1;
            }

            EndpointRouterBase* router
                = EndpointRouterFactory::instance().generate_object(
                        ep_router.value);

            const configure::WeightedRandomRenderConf &router_conf =
                conf.weighted_random_render_conf();
            if (!router || router->initialize(router_conf) != 0) {
                LOG(FATAL) << "Failed fetch valid ab test strategy"
                    << ", name:" << ep_router.value;
                return -1;
            }
            ep.ab_test = router;
        }

        // varlist
        uint32_t var_size = conf.variants_size();
        for (uint32_t vi = 0; vi < var_size; ++vi) {
            VariantInfo var;
            if (merge_variant(dft_var, conf.variants(vi),
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
        const configure::VariantConf& conf, VariantInfo& var) {
    try {
    // Connect
    const configure::ConnectionConf& conn = conf.connection_conf();

    PARSE_CONF_ITEM(conn, var.connection.tmo_conn,
            connect_timeout_ms, -1);
    PARSE_CONF_ITEM(conn, var.connection.tmo_rpc,
            rpc_timeout_ms, -1);
    PARSE_CONF_ITEM(conn, var.connection.tmo_hedge,
            hedge_request_timeout_ms, -1);
    PARSE_CONF_ITEM(conn, var.connection.cnt_retry_conn,
            connect_retry_count, -1);
    PARSE_CONF_ITEM(conn, var.connection.cnt_retry_hedge,
            hedge_fetch_retry_count, -1);
    PARSE_CONF_ITEM(conn, var.connection.cnt_maxconn_per_host,
            max_connection_per_host, -1);
    PARSE_CONF_ITEM(conn, var.connection.type_conn,
            connection_type, -1);

    // Naming
    const configure::NamingConf& name = conf.naming_conf();

    PARSE_CONF_ITEM(name, var.naminginfo.cluster_naming,
            cluster, -1);
    PARSE_CONF_ITEM(name, var.naminginfo.load_balancer,
            load_balance_strategy, -1);
    PARSE_CONF_ITEM(name, var.naminginfo.cluster_filter,
            cluster_filter_strategy, -1);

    // Rpc
    const configure::RpcParameter& params = conf.rpc_parameter();

    PARSE_CONF_ITEM(params, var.parameters.protocol,
            protocol, -1);
    PARSE_CONF_ITEM(params, var.parameters.compress_type,
            compress_type, -1);
    PARSE_CONF_ITEM(params, var.parameters.package_size,
            package_size, -1);
    PARSE_CONF_ITEM(params, var.parameters.max_channel,
            max_channel_per_request, -1);
    // Split
    const configure::SplitConf& splits = conf.split_conf();

    PARSE_CONF_ITEM(splits, var.splitinfo.split_tag,
            split_tag_name, -1);
    PARSE_CONF_ITEM(splits, var.splitinfo.tag_cands_str,
            tag_candidates, -1);
    if (parse_tag_values(var.splitinfo) != 0) {
        LOG(FATAL) << "Failed parse tag_values:" <<
            var.splitinfo.tag_cands_str.value;
        return -1;
    }

    // tag
    PARSE_CONF_ITEM(conf, var.parameters.route_tag,
            tag, -1);

    // router
    ConfigItem<std::string> var_router;
    PARSE_CONF_ITEM(conf, var_router, variant_router, -1);
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
        const configure::VariantConf& conf,
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
