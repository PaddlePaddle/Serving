// Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "core/sdk-cpp/include/config_manager.h"
#ifdef BCLOUD
#include "baidu/rpc/server.h"
#else
#include "brpc/server.h"
#endif
#include "core/sdk-cpp/include/abtest.h"

namespace baidu {
namespace paddle_serving {
namespace sdk_cpp {

using configure::SDKConf;

int EndpointConfigManager::create(const std::string& sdk_desc_str) {
  if (load(sdk_desc_str) != 0) {
    LOG(ERROR) << "Failed reload endpoint config";
    return -1;
  }

  return 0;
}

int EndpointConfigManager::create(const char* path, const char* file) {
  _endpoint_config_path = path;
  _endpoint_config_file = file;

  if (load() != 0) {
    LOG(ERROR) << "Failed reload endpoint config";
    return -1;
  }

  return 0;
}

int EndpointConfigManager::load(const std::string& sdk_desc_str) {
  try {
    SDKConf sdk_conf;
    sdk_conf.ParseFromString(sdk_desc_str);
    VariantInfo default_var;
    if (init_one_variant(sdk_conf.default_variant_conf(), default_var) != 0) {
      LOG(ERROR) << "Failed read default var conf";
      return -1;
    }

    uint32_t ep_size = sdk_conf.predictors_size();
    for (uint32_t ei = 0; ei < ep_size; ++ei) {
      EndpointInfo ep;
      if (init_one_endpoint(sdk_conf.predictors(ei), ep, default_var) != 0) {
        LOG(ERROR) << "Failed read endpoint info at: " << ei;
        return -1;
      }

      std::map<std::string, EndpointInfo>::iterator it;
      if (_ep_map.find(ep.endpoint_name) != _ep_map.end()) {
        LOG(ERROR) << "Cannot insert duplicated endpoint"
                   << ", ep name: " << ep.endpoint_name;
      }

      std::pair<std::map<std::string, EndpointInfo>::iterator, bool> r =
          _ep_map.insert(std::make_pair(ep.endpoint_name, ep));
      if (!r.second) {
        LOG(ERROR) << "Failed insert endpoint, name" << ep.endpoint_name;
        return -1;
      }
    }
  } catch (std::exception& e) {
    LOG(ERROR) << "Failed load configure" << e.what();
    return -1;
  }
  VLOG(2) << "Success reload endpoint config file, id: "
          << _current_endpointmap_id;
  return 0;
}

int EndpointConfigManager::load() {
  try {
    SDKConf sdk_conf;
    if (configure::read_proto_conf(_endpoint_config_path.c_str(),
                                   _endpoint_config_file.c_str(),
                                   &sdk_conf) != 0) {
      LOG(ERROR) << "Failed initialize endpoint list"
                 << ", config: " << _endpoint_config_path << "/"
                 << _endpoint_config_file;
      return -1;
    }

    VariantInfo default_var;
    if (init_one_variant(sdk_conf.default_variant_conf(), default_var) != 0) {
      LOG(ERROR) << "Failed read default var conf";
      return -1;
    }

    uint32_t ep_size = sdk_conf.predictors_size();
    for (uint32_t ei = 0; ei < ep_size; ++ei) {
      EndpointInfo ep;
      if (init_one_endpoint(sdk_conf.predictors(ei), ep, default_var) != 0) {
        LOG(ERROR) << "Failed read endpoint info at: " << ei;
        return -1;
      }

      std::map<std::string, EndpointInfo>::iterator it;
      if (_ep_map.find(ep.endpoint_name) != _ep_map.end()) {
        LOG(ERROR) << "Cannot insert duplicated endpoint"
                   << ", ep name: " << ep.endpoint_name;
      }

      std::pair<std::map<std::string, EndpointInfo>::iterator, bool> r =
          _ep_map.insert(std::make_pair(ep.endpoint_name, ep));
      if (!r.second) {
        LOG(ERROR) << "Failed insert endpoint, name" << ep.endpoint_name;
        return -1;
      }
    }
  } catch (std::exception& e) {
    LOG(ERROR) << "Failed load configure" << e.what();
    return -1;
  }
  VLOG(2) << "Success reload endpoint config file, id: "
          << _current_endpointmap_id;
  return 0;
}

int EndpointConfigManager::init_one_endpoint(const configure::Predictor& conf,
                                             EndpointInfo& ep,
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
      if (ep_router.value != "WeightedRandomRender") {
        LOG(ERROR) << "endpointer_router unrecognized " << ep_router.value;
        return -1;
      }

      EndpointRouterBase* router =
          EndpointRouterFactory::instance().generate_object(ep_router.value);

      const configure::WeightedRandomRenderConf& router_conf =
          conf.weighted_random_render_conf();
      if (!router || router->initialize(router_conf) != 0) {
        LOG(ERROR) << "Failed fetch valid ab test strategy"
                   << ", name:" << ep_router.value;
        return -1;
      }
      ep.ab_test = router;
    }

    // varlist
    uint32_t var_size = conf.variants_size();
    for (uint32_t vi = 0; vi < var_size; ++vi) {
      VariantInfo var;
      if (merge_variant(dft_var, conf.variants(vi), var) != 0) {
        LOG(ERROR) << "Failed merge variant info at: " << vi;
        return -1;
      }

      ep.vars.push_back(var);
    }

    if (ep.vars.size() > 1 && ep.ab_test == NULL) {
      LOG(ERROR) << "EndpointRouter must be configured, when"
                 << " #Variants > 1.";
      return -1;
    }

    VLOG(2) << "Succ load one endpoint, name: " << ep.endpoint_name
            << ", count of variants: " << ep.vars.size() << ".";
  } catch (std::exception& e) {
    LOG(ERROR) << "Exception acccurs when load endpoint conf"
               << ", message: " << e.what();
    return -1;
  }
  return 0;
}

int EndpointConfigManager::init_one_variant(const configure::VariantConf& conf,
                                            VariantInfo& var) {
  try {
    // Connect
    const configure::ConnectionConf& conn = conf.connection_conf();

    PARSE_CONF_ITEM(conn, var.connection.tmo_conn, connect_timeout_ms, -1);
    PARSE_CONF_ITEM(conn, var.connection.tmo_rpc, rpc_timeout_ms, -1);
    PARSE_CONF_ITEM(
        conn, var.connection.tmo_hedge, hedge_request_timeout_ms, -1);
    PARSE_CONF_ITEM(
        conn, var.connection.cnt_retry_conn, connect_retry_count, -1);
    PARSE_CONF_ITEM(
        conn, var.connection.cnt_retry_hedge, hedge_fetch_retry_count, -1);
    PARSE_CONF_ITEM(
        conn, var.connection.cnt_maxconn_per_host, max_connection_per_host, -1);
    PARSE_CONF_ITEM(conn, var.connection.type_conn, connection_type, -1);

    // Naming
    const configure::NamingConf& name = conf.naming_conf();

    PARSE_CONF_ITEM(name, var.naminginfo.cluster_naming, cluster, -1);
    PARSE_CONF_ITEM(
        name, var.naminginfo.load_balancer, load_balance_strategy, -1);
    PARSE_CONF_ITEM(
        name, var.naminginfo.cluster_filter, cluster_filter_strategy, -1);

    // Rpc
    const configure::RpcParameter& params = conf.rpc_parameter();

    PARSE_CONF_ITEM(params, var.parameters.protocol, protocol, -1);
    PARSE_CONF_ITEM(params, var.parameters.compress_type, compress_type, -1);
    PARSE_CONF_ITEM(params, var.parameters.package_size, package_size, -1);
    PARSE_CONF_ITEM(
        params, var.parameters.max_channel, max_channel_per_request, -1);
    // Split
    const configure::SplitConf& splits = conf.split_conf();

    PARSE_CONF_ITEM(splits, var.splitinfo.split_tag, split_tag_name, -1);
    PARSE_CONF_ITEM(splits, var.splitinfo.tag_cands_str, tag_candidates, -1);
    if (parse_tag_values(var.splitinfo) != 0) {
      LOG(ERROR) << "Failed parse tag_values:"
                 << var.splitinfo.tag_cands_str.value;
      return -1;
    }

    // tag
    PARSE_CONF_ITEM(conf, var.parameters.route_tag, tag, -1);
  } catch (...) {
    LOG(ERROR) << "Failed load variant from configure unit";
    return -1;
  }

  return 0;
}

int EndpointConfigManager::merge_variant(const VariantInfo& default_var,
                                         const configure::VariantConf& conf,
                                         VariantInfo& merged_var) {
  merged_var = default_var;

  return init_one_variant(conf, merged_var);
}

int EndpointConfigManager::parse_tag_values(SplitParameters& split) {
  split.tag_values.clear();
  if (!split.split_tag.init || !split.tag_cands_str.init) {
    VLOG(2) << "split info not set, skip...";
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
      tag_value_str = tag_str.substr(start_pos, end_pos - start_pos);
      start_pos = end_pos + 1;
    }

    split.tag_values.push_back(tag_value_str);
  } while (end_pos != std::string::npos);

  return 0;
}

}  // namespace sdk_cpp
}  // namespace paddle_serving
}  // namespace baidu

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
