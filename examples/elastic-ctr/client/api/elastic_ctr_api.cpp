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

#include "elastic-ctr/client/api/elastic_ctr_api.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <vector>

namespace baidu {
namespace paddle_serving {
namespace elastic_ctr {

const int VARIABLE_NAME_LEN = 256;

static void thread_resource_delete(void *d) {
#if 1
  LOG(INFO) << "thread_resource_delete on " << bthread_self();
#endif
  delete static_cast<ThreadResource *>(d);
}

std::set<std::string> ElasticCTRPredictorApi::slot_names_;

int ThreadResource::clear() {
  request_.clear_instances();
  response_.clear_predictions();

  for (auto it : instance_map_) {
    delete it.second;
  }
  return 0;
}

ReqInstance *ThreadResource::add_instance() {
  ReqInstance *instance = request_.add_instances();

  InstanceInfo *instance_info = new InstanceInfo();
  instance_map_[instance] = instance_info;

  return instance;
}

int ThreadResource::add_slot(ReqInstance *instance,
                             const std::string &slot_name,
                             uint64_t value) {
  auto instance_it = instance_map_.find(instance);
  if (instance_it == instance_map_.end()) {
    return -1;
  }

  InstanceInfo *instance_info = instance_it->second;

  auto slot_it = instance_info->slot_map_.find(slot_name);
  Slot *slot = NULL;

  if (slot_it == instance_info->slot_map_.end()) {
    slot = instance->add_slots();
    instance_info->slot_map_[slot_name] = slot;
  } else {
    slot = slot_it->second;
  }
  slot->set_slot_name(slot_name);
  slot->add_feasigns(value);

  return 0;
}

void ThreadResource::validate_request(const std::set<std::string> &slot_names) {
  for (auto it : instance_map_) {
    ReqInstance *req_instance = it.first;
    InstanceInfo *instance_info = it.second;

    for (auto slot_name : slot_names) {
      if (instance_info->slot_map_.find(slot_name) ==
          instance_info->slot_map_.end()) {
        LOG(INFO) << "Missing values for slot " << slot_name.c_str();
        add_slot(req_instance, slot_name, 0);
      }
    }
  }
}

int ElasticCTRPredictorApi::read_slot_conf(const char *path,
                                           const char *slot_conf_file) {
  struct stat stat_buf;
  char name[VARIABLE_NAME_LEN];
  snprintf(name, VARIABLE_NAME_LEN, "%s/%s", path, slot_conf_file);
  if (stat(name, &stat_buf) != 0) {
    LOG(ERROR) << "Error stating file" << name;
    return -1;
  }

  std::ifstream fs(name);
  for (std::string line; std::getline(fs, line);) {
    slot_names_.insert(line);
  }

#if 1
  for (auto x : slot_names_) {
    LOG(INFO) << "slot: " << x.c_str();
  }
#endif

  return 0;
}

int ElasticCTRPredictorApi::init(const char *path,
                                 const char *slot_conf_file,
                                 const char *serving_conf_file) {
  int ret = api_.create(path, serving_conf_file);
  if (ret != 0) {
    return ret;
  }

  ret = read_slot_conf(path, slot_conf_file);
  if (ret != 0) {
    return ret;
  }

  // Thread-local storage
  if (pthread_key_create(&tls_bspec_key_, thread_resource_delete) != 0) {
    LOG(ERROR) << "unable to create tls_bthread_key of thrd_data";
    return -1;
  }

  return 0;
}

int ElasticCTRPredictorApi::thrd_initialize() {
  api_.thrd_initialize();

  ThreadResource *thread_resource =
      reinterpret_cast<ThreadResource *>(pthread_getspecific(tls_bspec_key_));
  if (thread_resource == NULL) {
    thread_resource = new (std::nothrow) ThreadResource;

    if (thread_resource == NULL) {
      LOG(ERROR) << "failed to create thread local resource";
      return -1;
    }

    if (pthread_setspecific(tls_bspec_key_, thread_resource) != 0) {
      LOG(ERROR) << "unable to set tls thread local resource";
      delete thread_resource;
      thread_resource = NULL;
      return -1;
    }
  }

  return 0;
}

int ElasticCTRPredictorApi::thrd_clear() {
  api_.thrd_clear();

  ThreadResource *thread_resource =
      reinterpret_cast<ThreadResource *>(pthread_getspecific(tls_bspec_key_));
  if (thread_resource == NULL) {
    if (thread_resource == NULL) {
      LOG(ERROR) << "ERROR: thread local resource is null";
      return -1;
    }
  }

  if (thread_resource->clear() != 0) {
    LOG(ERROR) << "ElasticCTRPredictorApi: thrd_clear() fail";
  }

  return 0;
}

int ElasticCTRPredictorApi::thrd_finalize() {
  api_.thrd_finalize();
  return 0;
}

void ElasticCTRPredictorApi::destroy() {
  pthread_key_delete(tls_bspec_key_);
  return;
}

ReqInstance *ElasticCTRPredictorApi::add_instance() {
  ThreadResource *thread_resource =
      reinterpret_cast<ThreadResource *>(pthread_getspecific(tls_bspec_key_));
  if (thread_resource == NULL) {
    if (thread_resource == NULL) {
      LOG(ERROR) << "ERROR: thread local resource is null";
      return NULL;
    }
  }

  ReqInstance *instance = thread_resource->add_instance();
  return instance;
}

int ElasticCTRPredictorApi::add_slot(ReqInstance *instance,
                                     const std::string slot_name,
                                     int64_t value) {
  ThreadResource *thread_resource =
      reinterpret_cast<ThreadResource *>(pthread_getspecific(tls_bspec_key_));
  if (thread_resource == NULL) {
    if (thread_resource == NULL) {
      LOG(ERROR) << "ERROR: thread local resource is null";
      return -1;
    }
  }

  if (slot_names_.find(slot_name) == slot_names_.end()) {
    LOG(ERROR) << "Slot name not match with those in slot.conf: "
               << slot_name.c_str();
    return -1;
  }

  return thread_resource->add_slot(instance, slot_name, value);
}

void ElasticCTRPredictorApi::validate_request() {
  ThreadResource *thread_resource =
      reinterpret_cast<ThreadResource *>(pthread_getspecific(tls_bspec_key_));
  if (thread_resource == NULL) {
    if (thread_resource == NULL) {
      LOG(ERROR) << "ERROR: thread local resource is null";
      return;
    }
  }

  thread_resource->validate_request(slot_names_);
}

int ElasticCTRPredictorApi::inference(
    std::vector<std::vector<float>> &results_vec) {
  ThreadResource *thread_resource =
      reinterpret_cast<ThreadResource *>(pthread_getspecific(tls_bspec_key_));
  if (thread_resource == NULL) {
    if (thread_resource == NULL) {
      LOG(ERROR) << "ERROR: thread local resource is null";
      return -1;
    }
  }

  Predictor *predictor = api_.fetch_predictor("ctr_prediction_service");
  if (!predictor) {
    LOG(ERROR) << "Failed fetch predictor: ctr_prediction_service";
    return -1;
  }

  validate_request();

  int ret = predictor->inference(thread_resource->get_request(),
                                 thread_resource->get_response());
  if (ret != 0) {
    LOG(ERROR) << "Failed call predictor with req "
               << thread_resource->get_request()->ShortDebugString();
    return ret;
  }

  Response *response = thread_resource->get_response();

  for (int i = 0; i < response->predictions_size(); ++i) {
    const ResInstance &res_instance = response->predictions(i);
    std::vector<float> res;
    res.push_back(res_instance.prob0());
    res.push_back(res_instance.prob1());
    results_vec.push_back(res);
  }

  return 0;
}

}  // namespace elastic_ctr
}  // namespace paddle_serving
}  // namespace baidu
