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

#include "predictor/framework/resource.h"
#include <string>
#include "predictor/common/inner_common.h"
#include "predictor/framework/infer.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

using configure::ResourceConf;

// __thread bool p_thread_initialized = false;

static void dynamic_resource_deleter(void* d) {
#if 1
  LOG(INFO) << "dynamic_resource_delete on " << bthread_self();
#endif
  delete static_cast<DynamicResource*>(d);
}

DynamicResource::DynamicResource() {}

DynamicResource::~DynamicResource() {}

int DynamicResource::initialize() { return 0; }

int DynamicResource::clear() { return 0; }

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

  if (FLAGS_enable_model_toolkit) {
    int err = 0;
    std::string model_toolkit_path = resource_conf.model_toolkit_path();
    if (err != 0) {
      LOG(ERROR) << "read model_toolkit_path failed, path[" << path
                 << "], file[" << file << "]";
      return -1;
    }
    std::string model_toolkit_file = resource_conf.model_toolkit_file();
    if (err != 0) {
      LOG(ERROR) << "read model_toolkit_file failed, path[" << path
                 << "], file[" << file << "]";
      return -1;
    }
    if (InferManager::instance().proc_initialize(
            model_toolkit_path.c_str(), model_toolkit_file.c_str()) != 0) {
      LOG(ERROR) << "failed proc initialize modeltoolkit, config: "
                 << model_toolkit_path << "/" << model_toolkit_file;
      return -1;
    }
  }

  if (THREAD_KEY_CREATE(&_tls_bspec_key, dynamic_resource_deleter) != 0) {
    LOG(ERROR) << "unable to create tls_bthread_key of thrd_data";
    return -1;
  }
  THREAD_SETSPECIFIC(_tls_bspec_key, NULL);
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

  DynamicResource* p_dynamic_resource =
      reinterpret_cast<DynamicResource*>(THREAD_GETSPECIFIC(_tls_bspec_key));
  if (p_dynamic_resource == NULL) {
    p_dynamic_resource = new (std::nothrow) DynamicResource;
    if (p_dynamic_resource == NULL) {
      LOG(ERROR) << "failed to create tls DynamicResource";
      return -1;
    }
    if (p_dynamic_resource->initialize() != 0) {
      LOG(ERROR) << "DynamicResource initialize failed.";
      delete p_dynamic_resource;
      p_dynamic_resource = NULL;
      return -1;
    }

    if (THREAD_SETSPECIFIC(_tls_bspec_key, p_dynamic_resource) != 0) {
      LOG(ERROR) << "unable to set tls DynamicResource";
      delete p_dynamic_resource;
      p_dynamic_resource = NULL;
      return -1;
    }
  }
#if 0
    LOG(INFO) << "Successfully thread initialized dynamic resource";
#else
  LOG(INFO) << bthread_self()
            << ": Successfully thread initialized dynamic resource "
            << p_dynamic_resource;

#endif
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

  DynamicResource* p_dynamic_resource =
      reinterpret_cast<DynamicResource*>(THREAD_GETSPECIFIC(_tls_bspec_key));
  if (p_dynamic_resource == NULL) {
#if 0
    LOG(ERROR) << "tls dynamic resource shouldn't be null after "
        << "thread_initialize";
#else
    LOG(ERROR)
        << bthread_self()
        << ": tls dynamic resource shouldn't be null after thread_initialize";
#endif
    return -1;
  }
  if (p_dynamic_resource->clear() != 0) {
    LOG(ERROR) << "Failed to invoke dynamic resource clear";
    return -1;
  }

  LOG(INFO) << bthread_self() << "Resource::thread_clear success";
  // ...
  return 0;
}

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

  THREAD_KEY_DELETE(_tls_bspec_key);

  return 0;
}

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
