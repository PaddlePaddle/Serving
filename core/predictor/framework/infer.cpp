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

#include "core/predictor/framework/infer.h"

namespace baidu {
namespace paddle_serving {
namespace predictor {

int ReloadableInferEngine::proc_initialize_impl(
    const configure::EngineDesc& conf, bool version) {
  _reload_tag_file = conf.reloadable_meta();
  _reload_type = conf.reloadable_type();
  _model_dir = conf.model_dir();
  _infer_thread_num = conf.runtime_thread_num();
  _infer_batch_size = conf.batch_infer_size();
  _infer_overrun = conf.enable_overrun();
  _allow_split_request = conf.allow_split_request();

  _conf = conf;

  if (!check_need_reload() || load(conf) != 0) {
    LOG(ERROR) << "Failed load model_data_path" << _model_dir;
    return -1;
  }

  if (parse_version_info(conf, version) != 0) {
    LOG(ERROR) << "Failed parse version info";
    return -1;
  }

  LOG(WARNING) << "Succ load model:" << _model_dir;
  return 0;
}

int ReloadableInferEngine::proc_initialize(const configure::EngineDesc& conf,
                                           bool version) {
  if (proc_initialize_impl(conf, version) != 0) {
    LOG(ERROR) << "Failed proc initialize impl";
    return -1;
  }

  // init bsf framework
  if (_infer_thread_num <= 0) {
    return 0;
  }

  // init bsf framework
  im::bsf::TaskExecutorVector<TaskT>::instance()[_model_index]
      .set_thread_init_fn(
          boost::bind(&InferEngine::thrd_initialize_impl, this));
  im::bsf::TaskExecutorVector<TaskT>::instance()[_model_index]
      .set_thread_reset_fn(boost::bind(&InferEngine::thrd_clear_impl, this));
  im::bsf::TaskExecutorVector<TaskT>::instance()[_model_index]
      .set_thread_callback_fn(
          boost::bind(&InferEngine::task_infer_impl, this, _1, _2));
  im::bsf::TaskExecutorVector<TaskT>::instance()[_model_index].set_batch_size(
      _infer_batch_size);
  im::bsf::TaskExecutorVector<TaskT>::instance()[_model_index].set_overrun(
      _infer_overrun);
  im::bsf::TaskExecutorVector<TaskT>::instance()[_model_index]
      .set_allow_split_request(_allow_split_request);
  if (im::bsf::TaskExecutorVector<TaskT>::instance()[_model_index].start(
          _infer_thread_num) != 0) {
    LOG(ERROR) << "Failed start bsf executor, threads:" << _infer_thread_num;
    return -1;
  }

  LOG(WARNING) << "Enable batch schedule framework, thread_num:"
               << _infer_thread_num << ", batch_size:" << _infer_batch_size
               << ", enable_overrun:" << _infer_overrun
               << ", allow_split_request:" << _allow_split_request;
  return 0;
}

// Multiple threads will enter this method of the same object
// One Model corresponds to One ReloadableInferEngine object.
// ReloadableInferEngine object is Process object.
// One ReloadableInferEngine object can have several ModelData<EngineCore>
// ModelData<EngineCore> is Thread object.
int ReloadableInferEngine::infer(const void* in,
                                 void* out,
                                 uint32_t batch_size) {
  if (_infer_thread_num <= 0) {
    return infer_impl(in, out, batch_size);
  }

  im::bsf::TaskManager<paddle::PaddleTensor, paddle::PaddleTensor> task_manager(
      _model_index);
  task_manager.schedule(in, out, MempoolWrapper::instance().get_thread_memory_ptr());
  task_manager.wait();
  return 0;
}

int ReloadableInferEngine::thrd_initialize() {
  if (_infer_thread_num > 0) {
    return 0;
  }
  return thrd_initialize_impl();
}

int ReloadableInferEngine::thrd_clear() {
  if (_infer_thread_num > 0) {
    return 0;
  }
  return thrd_clear_impl();
}

int ReloadableInferEngine::proc_finalize() {
  if (proc_finalize_impl() != 0) {
    LOG(ERROR) << "Failed proc finalize impl";
    return -1;
  }

  if (_infer_thread_num > 0) {
    im::bsf::TaskExecutorVector<TaskT>::instance()[_model_index].stop();
  }
  return 0;
}

int ReloadableInferEngine::reload() {
  if (check_need_reload()) {
    LOG(WARNING) << "begin reload model[" << _model_dir << "].";
    return load(_conf);
  }
  return 0;
}

int ReloadableInferEngine::parse_version_info(
    const configure::EngineDesc& config, bool version) {
  _version = uint64_t(-1);
  return 0;
}

bool ReloadableInferEngine::check_need_reload() {
  if (_reload_type == "timestamp_ne") {
    return check_timestamp_ne();
  } else if (_reload_type == "timestamp_gt") {
    return check_timestamp_gt();
  } else if (_reload_type == "md5sum") {
    return check_md5sum();
  } else if (_reload_type == "revision") {
    return check_revision();
  } else if (_reload_type == "none") {
    return false;
  }

  LOG(ERROR) << "Not support reload type: " << _reload_type;
  return false;
}

bool ReloadableInferEngine::check_timestamp_ne() {
  struct stat st;
  if (stat(_reload_tag_file.c_str(), &st) != 0) {
    LOG(ERROR) << "Failed stat config file:" << _reload_tag_file;
    return false;
  }

  if ((st.st_mode & S_IFREG) && st.st_mtime != _last_record.timestamp) {
    _last_record.timestamp = st.st_mtime;
    return true;
  }

  return false;
}

bool ReloadableInferEngine::check_timestamp_gt() {
  struct stat st;
  if (stat(_reload_tag_file.c_str(), &st) != 0) {
    LOG(ERROR) << "Failed stat config file:" << _reload_tag_file;
    return false;
  }

  if ((st.st_mode & S_IFREG) && st.st_mtime > _last_record.timestamp) {
    _last_record.timestamp = st.st_mtime;
    return true;
  }

  return false;
}

int VersionedInferEngine::proc_initialize(const configure::EngineDesc& conf) {
  if (proc_initialize(conf, false) != 0) {
    LOG(ERROR) << "Failed proc intialize engine: " << conf.name().c_str();
    return -1;
  }

  LOG(WARNING) << "Succ proc initialize engine: " << conf.name().c_str();
  return 0;
}

int VersionedInferEngine::proc_initialize(const configure::EngineDesc& conf,
                                          bool version) {
  std::string engine_type = conf.type();
  InferEngine* engine =
      StaticInferFactory::instance().generate_object(engine_type);
  engine->set_model_index(_model_index);
  if (!engine) {
    LOG(ERROR) << "Failed generate engine with type:" << engine_type;
    return -1;
  }
#ifndef BCLOUD
  VLOG(2) << "FLAGS_logtostderr " << FLAGS_logtostderr;
  int tmp = FLAGS_logtostderr;
  if (engine->proc_initialize(conf, version) != 0) {
    LOG(ERROR) << "Failed initialize engine, type:" << engine_type;
    return -1;
  }
  VLOG(2) << "FLAGS_logtostderr " << FLAGS_logtostderr;
  FLAGS_logtostderr = tmp;
#else
  if (engine->proc_initialize(conf, version) != 0) {
    LOG(ERROR) << "Failed initialize engine, type:" << engine_type;
    return -1;
  }
#endif
  auto r = _versions.insert(std::make_pair(engine->version(), engine));
  if (!r.second) {
    LOG(ERROR) << "Failed insert item: " << engine->version()
               << ", type: " << engine_type;
    return -1;
  }
  LOG(WARNING) << "Succ proc initialize version engine: " << engine->version();
  return 0;
}

int VersionedInferEngine::proc_finalize() {
  for (auto iter = _versions.begin(); iter != _versions.end(); ++iter) {
    if (iter->second->proc_finalize() != 0) {
      LOG(ERROR) << "Failed proc finalize version engine: " << iter->first;
    }
    LOG(WARNING) << "Succ proc finalize version engine: " << iter->first;
  }
  return 0;
}

int VersionedInferEngine::thrd_initialize() {
  for (auto iter = _versions.begin(); iter != _versions.end(); ++iter) {
    if (iter->second->thrd_initialize() != 0) {
      LOG(ERROR) << "Failed thrd initialize version engine: " << iter->first;
      return -1;
    }
    LOG(WARNING) << "Succ thrd initialize version engine: " << iter->first;
  }
  return 0;
}

int VersionedInferEngine::thrd_clear() {
  for (auto iter = _versions.begin(); iter != _versions.end(); ++iter) {
    if (iter->second->thrd_clear() != 0) {
      LOG(ERROR) << "Failed thrd clear version engine: " << iter->first;
      return -1;
    }
  }
  return 0;
}

int VersionedInferEngine::thrd_finalize() {
  for (auto iter = _versions.begin(); iter != _versions.end(); ++iter) {
    if (iter->second->thrd_finalize() != 0) {
      LOG(ERROR) << "Failed thrd finalize version engine: " << iter->first;
      return -1;
    }
    LOG(WARNING) << "Succ thrd finalize version engine: " << iter->first;
  }
  return 0;
}

int VersionedInferEngine::reload() {
  for (auto iter = _versions.begin(); iter != _versions.end(); ++iter) {
    if (iter->second->reload() != 0) {
      LOG(ERROR) << "Failed reload version engine: " << iter->first;
      return -1;
    }
    LOG(WARNING) << "Succ reload version engine: " << iter->first;
  }
  return 0;
}

uint64_t VersionedInferEngine::version() const {
  InferEngine* engine = default_engine();
  if (engine) {
    return engine->version();
  } else {
    return uint64_t(-1);
  }
}

// inference interface
InferEngine* VersionedInferEngine::default_engine() const {
  if (_versions.size() != 1) {
    LOG(ERROR) << "Ambiguous default engine version:" << _versions.size();
    return NULL;
  }

  return _versions.begin()->second;
}

int VersionedInferEngine::infer(const void* in,
                                void* out,
                                uint32_t batch_size) {
  InferEngine* engine = default_engine();
  if (!engine) {
    LOG(WARNING) << "fail to get default engine";
    return -1;
  }
  return engine->infer(in, out, batch_size);
}

// versioned inference interface
int VersionedInferEngine::infer(const void* in,
                                void* out,
                                uint32_t batch_size,
                                uint64_t version) {
  auto iter = _versions.find(version);
  if (iter == _versions.end()) {
    LOG(ERROR) << "Not found version engine: " << version;
    return -1;
  }

  return iter->second->infer(in, out, batch_size);
}

template <typename T>
T* VersionedInferEngine::get_core() {
  InferEngine* engine = default_engine();
  if (!engine) {
    LOG(WARNING) << "fail to get core";
    return NULL;
  }
  auto db_engine = dynamic_cast<DBReloadableInferEngine<T>*>(engine);
  if (db_engine) {
    return db_engine->get_core();
  }
  LOG(WARNING) << "fail to get core";
  return NULL;
}

template <typename T>
T* VersionedInferEngine::get_core(const uint64_t version) {
  auto iter = _versions.find(version);
  if (iter == _versions.end()) {
    LOG(ERROR) << "Not found version engine: " << version;
    return NULL;
  }

  auto db_engine = dynamic_cast<DBReloadableInferEngine<T>*>(iter->second);
  if (db_engine) {
    return db_engine->get_core();
  }
  LOG(WARNING) << "fail to get core for " << version;
  return NULL;
}

CubeCache* VersionedInferEngine::get_cube_cache() {
  InferEngine* engine = default_engine();
  if (!engine) {
    LOG(WARNING) << "fail to get default engine";
    return nullptr;
  }
  return engine->get_cube_cache();
}

int VersionedInferEngine::proc_initialize_impl(
    const configure::EngineDesc& conf, bool) {
  return -1;
}

int VersionedInferEngine::thrd_initialize_impl() { return -1; }
int VersionedInferEngine::thrd_finalize_impl() { return -1; }
int VersionedInferEngine::thrd_clear_impl() { return -1; }
int VersionedInferEngine::proc_finalize_impl() { return -1; }
int VersionedInferEngine::infer_impl(const void* in,
                                     void* out,
                                     uint32_t batch_size) {
  return -1;
}
int VersionedInferEngine::task_infer_impl(const void* in,
                                          void* out) {  // NOLINT
  return -1;
}

int InferManager::set_taskexecutor_num(size_t total_engine_num) {
  im::bsf::TaskExecutorVector<TaskT>::instance().resize(total_engine_num);
  return 0;
}

int InferManager::proc_initialize(const char* path,
                                  const char* file,
                                  std::shared_ptr<int> engine_index_ptr) {
  ModelToolkitConf model_toolkit_conf;
  if (configure::read_proto_conf(path, file, &model_toolkit_conf) != 0) {
    LOG(ERROR) << "failed load infer config, path: " << path << "/" << file;
    return -1;
  }
  uint32_t engine_num = model_toolkit_conf.engines_size();
  for (uint32_t ei = 0; ei < engine_num; ++ei) {
    LOG(INFO) << "model_toolkit_conf.engines(" << ei
              << ").name: " << model_toolkit_conf.engines(ei).name();
    std::string engine_name = model_toolkit_conf.engines(ei).name();
    VersionedInferEngine* engine = new (std::nothrow) VersionedInferEngine();
    int temp_engine_index_ptr = *engine_index_ptr;
    engine->set_model_index(temp_engine_index_ptr);
    *engine_index_ptr = temp_engine_index_ptr + 1;
    if (!engine) {
      LOG(ERROR) << "Failed generate versioned engine: " << engine_name;
      return -1;
    }
    if (engine->proc_initialize(model_toolkit_conf.engines(ei)) != 0) {
      LOG(ERROR) << "Failed initialize version engine, name:" << engine_name;
      return -1;
    }
    auto r = _map.insert(std::make_pair(engine_name, engine));
    if (!r.second) {
      LOG(ERROR) << "Failed insert item: " << engine_name;
      return -1;
    }
    LOG(WARNING) << "Succ proc initialize engine: " << engine_name;
  }
  return 0;
}

int InferManager::thrd_initialize() {
  for (auto it = _map.begin(); it != _map.end(); ++it) {
    if (it->second->thrd_initialize() != 0) {
      LOG(ERROR) << "Failed thrd initialize engine, name: " << it->first;
      return -1;
    }
    LOG(WARNING) << "Succ thrd initialize engine, name: " << it->first;
  }
  return 0;
}

int InferManager::thrd_clear() {
  for (auto it = _map.begin(); it != _map.end(); ++it) {
    if (it->second->thrd_clear() != 0) {
      LOG(ERROR) << "Failed thrd clear engine, name: " << it->first;
      return -1;
    }
  }
  return 0;
}

int InferManager::reload() {
  for (auto it = _map.begin(); it != _map.end(); ++it) {
    if (it->second->reload() != 0) {
      LOG(ERROR) << "Failed reload engine, name: " << it->first;
      return -1;
    }
  }
  return 0;
}

int InferManager::thrd_finalize() {
  for (auto it = _map.begin(); it != _map.end(); ++it) {
    if (it->second->thrd_finalize() != 0) {
      LOG(ERROR) << "Failed thrd finalize engine, name: " << it->first;
      return -1;
    }
    LOG(WARNING) << "Succ thrd finalize engine, name: " << it->first;
  }
  return 0;
}

int InferManager::proc_finalize() {
  for (auto it = _map.begin(); it != _map.end(); ++it) {
    if (it->second->proc_finalize() != 0) {
      LOG(ERROR) << "Failed proc finalize engine, name: " << it->first;
      return -1;
    }
    LOG(WARNING) << "Succ proc finalize engine, name: " << it->first;
  }
  _map.clear();
  return 0;
}

// Inference interface
int InferManager::infer(const char* model_name,
                        const void* in,
                        void* out,
                        uint32_t batch_size) {
  auto it = _map.find(model_name);
  if (it == _map.end()) {
    LOG(WARNING) << "Cannot find engine in map, model name:" << model_name;
    return -1;
  }
  return it->second->infer(in, out, batch_size);
}

template <typename T>
T* InferManager::get_core(const char* model_name) {
  auto it = _map.find(model_name);
  if (it == _map.end()) {
    LOG(WARNING) << "Cannot find engine in map, model name:" << model_name;
    return NULL;
  }
  auto infer_engine =
      dynamic_cast<DBReloadableInferEngine<T>*>(it->second->default_engine());
  if (infer_engine) {
    return infer_engine->get_core();
  }
  LOG(WARNING) << "fail to get core for " << model_name;
  return NULL;
}

CubeCache* InferManager::get_cube_cache(const char* model_name) {
  auto it = _map.find(model_name);
  if (it == _map.end()) {
    LOG(WARNING) << "Cannot find engine in map, model name:" << model_name;
    return nullptr;
  }
  return it->second->get_cube_cache();
}

// Versioned inference interface
int InferManager::infer(const char* model_name,
                        const void* in,
                        void* out,
                        uint32_t batch_size,
                        uint64_t version) {
  auto it = _map.find(model_name);
  if (it == _map.end()) {
    LOG(WARNING) << "Cannot find engine in map, model name:" << model_name;
    return -1;
  }
  return it->second->infer(in, out, batch_size, version);
}

template <typename T>
T* InferManager::get_core(const char* model_name, const uint64_t version) {
  auto it = _map.find(model_name);
  if (it == _map.end()) {
    LOG(WARNING) << "Cannot find engine in map, model name:" << model_name;
    return NULL;
  }
  return it->second->get_core<T>(version);
}

int InferManager::query_version(const std::string& model, uint64_t& version) {
  auto it = _map.find(model);
  if (it == _map.end()) {
    LOG(WARNING) << "Cannot find engine in map, model name:" << model;
    return -1;
  }
  auto infer_engine = it->second->default_engine();
  if (!infer_engine) {
    LOG(WARNING) << "Cannot get default engine for model:" << model;
    return -1;
  }
  version = infer_engine->version();
  LOG(INFO) << "Succ get version: " << version << " for model: " << model;
  return 0;
}

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
