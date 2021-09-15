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

#include "core/predictor/framework/resource.h"
#include <sstream>
#include <string>
#include "core/predictor/common/inner_common.h"
#include "core/predictor/framework/kv_manager.h"
#ifdef BCLOUD
#include "aipe_sec_client.h"  // NOLINT
#endif
namespace baidu {
namespace paddle_serving {
namespace predictor {

using configure::ResourceConf;
using configure::GeneralModelConfig;
using rec::mcube::CubeAPI;

std::vector<std::shared_ptr<PaddleGeneralModelConfig>>
Resource::get_general_model_config() {
  return _configs;
}

void Resource::print_general_model_config(
    const std::shared_ptr<PaddleGeneralModelConfig>& config) {
  if (config == nullptr) {
    LOG(INFO) << "paddle general model config is not set";
    return;
  }
  LOG(INFO) << "Number of Feed Tensor: " << config->_feed_name.size();
  std::ostringstream oss;
  LOG(INFO) << "Feed Name Info";
  for (auto& feed_name : config->_feed_name) {
    oss << feed_name << " ";
  }
  LOG(INFO) << oss.str();
  oss.clear();
  oss.str("");
  LOG(INFO) << "Feed Type Info";
  for (auto& feed_type : config->_feed_type) {
    oss << feed_type << " ";
  }
  LOG(INFO) << oss.str();
  oss.clear();
  oss.str("");
  LOG(INFO) << "Lod Type Info";

  for (auto is_lod : config->_is_lod_feed) {
    oss << is_lod << " ";
  }

  LOG(INFO) << oss.str();
  oss.clear();
  oss.str("");
  LOG(INFO) << "Capacity Info";
  for (auto& cap : config->_capacity) {
    oss << cap << " ";
  }
  LOG(INFO) << oss.str();
  oss.clear();
  oss.str("");
  LOG(INFO) << "Feed Shape Info";
  int tensor_idx = 0;
  for (auto& shape : config->_feed_shape) {
    for (auto& dim : shape) {
      oss << dim << " ";
    }
    LOG(INFO) << "Tensor[" << tensor_idx++ << "].shape: " << oss.str();
    oss.clear();
    oss.str("");
  }
}

int Resource::initialize(const std::string& path, const std::string& file) {
  ResourceConf resource_conf;
  if (configure::read_proto_conf(path, file, &resource_conf) != 0) {
    LOG(ERROR) << "Failed initialize resource from: " << path << "/" << file;
    return -1;
  }

  // mempool
  if (MempoolWrapper::instance().initialize() != 0) {
    LOG(ERROR) << "Failed proc initialized mempool wrapper";
    return -1;
  }
  LOG(WARNING) << "Successfully proc initialized mempool wrapper";

#ifdef WITH_AUTH
  std::string product_name_str = resource_conf.auth_product_name();
  std::string container_id_str = resource_conf.auth_container_id();

  char* product_name = new char[product_name_str.size() + 1];
  snprintf(product_name,
           product_name_str.size() + 1,
           "%s",
           product_name_str.c_str());
  char* container_id = new char[container_id_str.size() + 1];
  snprintf(container_id,
           container_id_str.size() + 1,
           "%s",
           container_id_str.c_str());

  aipe_auth_request request;
  request.product_name = product_name;
  request.container_id = container_id;
  request.request_ts = (int64_t)time(NULL);

  LOG(INFO) << "\nEasypack info"
            << "\nproduct name: " << request.product_name
            << "\ncontainer_id: " << request.container_id
            << "\nrequest time stamp: " << request.request_ts;

  aipe_auth_response response;
  response = check_auth(request);

  if (response.result == 0) {
    LOG(INFO) << "Authentication succeed.";
  } else {
    LOG(ERROR) << "Authentication failed. Error code: " << response.result;
    return -1;
  }
#endif

  if (FLAGS_enable_model_toolkit) {
    size_t model_toolkit_num = resource_conf.model_toolkit_path_size();
    // 此处暂时认为，每个model_toolkit仅包含一个engine
    // 故认为 model_toolkit_num == engine总数
    // 若以后出现model_toolkit仅包含多个engine
    // 则应先for循环统计engine总数,再set_taskexecutor_num
    // 切不可动态im::bsf::TaskExecutorVector<TaskT>::instance().resize
    // TaskExecutor是线程池，内含锁，在engine进程初始化时已开始work加锁循环运行了
    // 之后再resize内存搬运，会导致work使用原锁，而搬运后的TaskExecutor的锁内存已改变
    if (InferManager::instance().set_taskexecutor_num(model_toolkit_num) != 0) {
      LOG(ERROR) << "failed set_taskexecutor_num";
      return -1;
    }
    std::shared_ptr<int> engine_index_ptr(new int(0));
    for (size_t mi = 0; mi < model_toolkit_num; ++mi) {
      std::string model_toolkit_path = resource_conf.model_toolkit_path(mi);
      std::string model_toolkit_file = resource_conf.model_toolkit_file(mi);

      if (InferManager::instance().proc_initialize(model_toolkit_path.c_str(),
                                                   model_toolkit_file.c_str(),
                                                   engine_index_ptr) != 0) {
        LOG(ERROR) << "failed proc initialize modeltoolkit, config: "
                   << model_toolkit_path << "/" << model_toolkit_file;
        return -1;
      }

      if (KVManager::instance().proc_initialize(
              model_toolkit_path.c_str(), model_toolkit_file.c_str()) != 0) {
        LOG(ERROR) << "Failed proc initialize kvmanager, config: "
                   << model_toolkit_path << "/" << model_toolkit_file;
      }
    }
  }

  // init rocksDB or cube instance
  if (resource_conf.has_cube_config_file() &&
      resource_conf.has_cube_config_path()) {
    LOG(INFO) << "init cube client, path[ " << resource_conf.cube_config_path()
              << " ], config file [ " << resource_conf.cube_config_file()
              << " ].";
    rec::mcube::CubeAPI* cube = rec::mcube::CubeAPI::instance();
    std::string cube_config_fullpath = "./" + resource_conf.cube_config_path() +
                                       "/" + resource_conf.cube_config_file();
    this->_cube_config_fullpath = cube_config_fullpath;
    this->_cube_quant_bits = resource_conf.has_cube_quant_bits()
                                 ? resource_conf.cube_quant_bits()
                                 : 0;
    if (this->_cube_quant_bits != 0 && this->_cube_quant_bits != 8) {
      LOG(ERROR) << "Cube quant bits illegal! should be 0 or 8.";
      return -1;
    }
    if (this->_cube_quant_bits == 0) {
      LOG(INFO) << "cube quant mode OFF";
    } else {
      LOG(INFO) << "cube quant mode ON, quant bits: " << this->_cube_quant_bits;
    }
  }

  THREAD_SETSPECIFIC(_tls_bspec_key, NULL);
  return 0;
}

// model config
int Resource::general_model_initialize(const std::string& path,
                                       const std::string& file) {
  if (this->_cube_config_fullpath.size() != 0) {
    LOG(INFO) << "init cube by config file : " << this->_cube_config_fullpath;
    rec::mcube::CubeAPI* cube = rec::mcube::CubeAPI::instance();
    int ret = cube->init(this->_cube_config_fullpath.c_str());
    if (ret != 0) {
      LOG(ERROR) << "cube init error";
      return -1;
    }
  }
  VLOG(2) << "general model path: " << path;
  VLOG(2) << "general model file: " << file;
  if (!FLAGS_enable_general_model) {
    LOG(ERROR) << "general model is not enabled";
    return -1;
  }
  ResourceConf resource_conf;
  if (configure::read_proto_conf(path, file, &resource_conf) != 0) {
    LOG(ERROR) << "Failed initialize resource from: " << path << "/" << file;
    return -1;
  }
  size_t general_model_num = resource_conf.general_model_path_size();
  for (size_t gi = 0; gi < general_model_num; ++gi) {
    std::string general_model_path = resource_conf.general_model_path(gi);
    std::string general_model_file = resource_conf.general_model_file(gi);

    GeneralModelConfig model_config;
    if (configure::read_proto_conf(general_model_path.c_str(),
                                   general_model_file.c_str(),
                                   &model_config) != 0) {
      LOG(ERROR) << "Failed initialize model config from: "
                 << general_model_path << "/" << general_model_file;
      return -1;
    }
    auto _config = std::make_shared<PaddleGeneralModelConfig>();
    int feed_var_num = model_config.feed_var_size();
    VLOG(2) << "load general model config";
    VLOG(2) << "feed var num: " << feed_var_num;
    _config->_feed_name.resize(feed_var_num);
    _config->_feed_alias_name.resize(feed_var_num);
    _config->_feed_type.resize(feed_var_num);
    _config->_is_lod_feed.resize(feed_var_num);
    _config->_capacity.resize(feed_var_num);
    _config->_feed_shape.resize(feed_var_num);
    for (int i = 0; i < feed_var_num; ++i) {
      _config->_feed_name[i] = model_config.feed_var(i).name();
      _config->_feed_alias_name[i] = model_config.feed_var(i).alias_name();
      VLOG(2) << "feed var[" << i << "]: " << _config->_feed_name[i];
      VLOG(2) << "feed var[" << i << "]: " << _config->_feed_alias_name[i];
      _config->_feed_type[i] = model_config.feed_var(i).feed_type();
      VLOG(2) << "feed type[" << i << "]: " << _config->_feed_type[i];

      if (model_config.feed_var(i).is_lod_tensor()) {
        VLOG(2) << "var[" << i << "] is lod tensor";
        _config->_feed_shape[i] = {-1};
        _config->_is_lod_feed[i] = true;
      } else {
        VLOG(2) << "var[" << i << "] is tensor";
        _config->_capacity[i] = 1;
        _config->_is_lod_feed[i] = false;
        for (int j = 0; j < model_config.feed_var(i).shape_size(); ++j) {
          int32_t dim = model_config.feed_var(i).shape(j);
          VLOG(2) << "var[" << i << "].shape[" << i << "]: " << dim;
          _config->_feed_shape[i].push_back(dim);
          _config->_capacity[i] *= dim;
        }
      }
    }

    int fetch_var_num = model_config.fetch_var_size();
    _config->_is_lod_fetch.resize(fetch_var_num);
    _config->_fetch_name.resize(fetch_var_num);
    _config->_fetch_alias_name.resize(fetch_var_num);
    _config->_fetch_shape.resize(fetch_var_num);
    for (int i = 0; i < fetch_var_num; ++i) {
      _config->_fetch_name[i] = model_config.fetch_var(i).name();
      _config->_fetch_alias_name[i] = model_config.fetch_var(i).alias_name();
      _config->_fetch_name_to_index[_config->_fetch_name[i]] = i;
      _config->_fetch_alias_name_to_index[_config->_fetch_alias_name[i]] = i;
      if (model_config.fetch_var(i).is_lod_tensor()) {
        VLOG(2) << "fetch var[" << i << "] is lod tensor";
        _config->_fetch_shape[i] = {-1};
        _config->_is_lod_fetch[i] = true;
      } else {
        _config->_is_lod_fetch[i] = false;
        for (int j = 0; j < model_config.fetch_var(i).shape_size(); ++j) {
          int dim = model_config.fetch_var(i).shape(j);
          _config->_fetch_shape[i].push_back(dim);
        }
      }
    }
    _configs.push_back(std::move(_config));
  }
  return 0;
}

int Resource::thread_initialize() {
  // mempool
  if (MempoolWrapper::instance().thread_initialize() != 0) {
    LOG(ERROR) << "Failed thread initialized mempool wrapper";
    return -1;
  }
  LOG(WARNING) << "Successfully thread initialized mempool wrapper";

  // infer manager
  if (FLAGS_enable_model_toolkit &&
      InferManager::instance().thrd_initialize() != 0) {
    LOG(ERROR) << "Failed thrd initialized infer manager";
    return -1;
  }

  return 0;
}

int Resource::thread_clear() {
  // mempool
  if (MempoolWrapper::instance().thread_clear() != 0) {
    LOG(ERROR) << "Failed thread clear mempool wrapper";
    return -1;
  }

  // infer manager
  if (FLAGS_enable_model_toolkit &&
      InferManager::instance().thrd_clear() != 0) {
    LOG(ERROR) << "Failed thrd clear infer manager";
    return -1;
  }
  return 0;
}
size_t Resource::get_cube_quant_bits() { return this->_cube_quant_bits; }

int Resource::reload() {
  if (FLAGS_enable_model_toolkit && InferManager::instance().reload() != 0) {
    LOG(ERROR) << "Failed reload infer manager";
    return -1;
  }

  // other resource reload here...
  return 0;
}

int Resource::finalize() {
  if (FLAGS_enable_model_toolkit &&
      InferManager::instance().proc_finalize() != 0) {
    LOG(ERROR) << "Failed proc finalize infer manager";
    return -1;
  }
  if (CubeAPI::instance()->destroy() != 0) {
    LOG(ERROR) << "Destory cube api failed ";
    return -1;
  }
  THREAD_KEY_DELETE(_tls_bspec_key);

  return 0;
}

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
