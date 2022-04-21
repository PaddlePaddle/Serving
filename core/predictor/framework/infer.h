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

#pragma once
#include <pthread.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <functional>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>
#include "core/predictor/common/inner_common.h"
#include "core/predictor/framework/bsf.h"
#include "core/predictor/framework/cache.h"
#include "core/predictor/framework/factory.h"
#include "core/predictor/framework/infer_data.h"
#include "core/predictor/framework/memory.h"
#include "core/predictor/framework/predictor_metric.h"
#include "paddle_inference_api.h"  // NOLINT
//#include "experimental/float16.h"
#include "experimental/phi/common/float16.h"
namespace baidu {
namespace paddle_serving {
namespace predictor {

using configure::ModelToolkitConf;

// Auto mutex lock
class AutoLock {
 public:
  explicit AutoLock(pthread_mutex_t& mutex) : _mut(mutex) {
    pthread_mutex_lock(&mutex);
  }
  ~AutoLock() { pthread_mutex_unlock(&_mut); }

 private:
  pthread_mutex_t& _mut;
};

// Gloabl singleton mutex lock
class GlobalCreateMutex {
 public:
  pthread_mutex_t& mutex() { return _mut; }

  static pthread_mutex_t& instance() {
    static GlobalCreateMutex gmutex;
    return gmutex.mutex();
  }

 private:
  GlobalCreateMutex() { pthread_mutex_init(&_mut, NULL); }
  pthread_mutex_t _mut;
};

// InferEngine
class InferEngine {
 public:
  virtual ~InferEngine() {}

  virtual int proc_initialize(const configure::EngineDesc& conf, bool version) {
    return proc_initialize_impl(conf, version);
  }
  virtual int proc_finalize() { return proc_finalize_impl(); }
  virtual int thrd_initialize() { return thrd_initialize_impl(); }
  virtual int thrd_clear() { return thrd_clear_impl(); }
  virtual int thrd_finalize() { return thrd_finalize_impl(); }
  virtual int infer(const void* in, void* out, uint32_t batch_size = -1) {
    return infer_impl(in, out, batch_size);
  }
  virtual void set_model_index(uint32_t index) { _model_index = index; }
  virtual int reload() = 0;

  virtual uint64_t version() const = 0;

  // begin: framework inner call
  virtual int proc_initialize_impl(const configure::EngineDesc& conf,
                                   bool version) = 0;
  virtual int thrd_initialize_impl() = 0;
  virtual int thrd_finalize_impl() = 0;
  virtual int thrd_clear_impl() = 0;
  virtual int proc_finalize_impl() = 0;
  virtual int infer_impl(const void* in,
                         void* out,
                         uint32_t batch_size = -1) = 0;
  virtual int task_infer_impl(const void* in, void* out) = 0;  // NOLINT
  virtual CubeCache* get_cube_cache() = 0;

 protected:
  uint32_t _model_index;
  // end: framework inner call
};

typedef im::bsf::Task<paddle::PaddleTensor, paddle::PaddleTensor> TaskT;
class ReloadableInferEngine : public InferEngine {
 public:
  virtual ~ReloadableInferEngine() {}

  // Reloadable record
  union ReloadableRecord {
    time_t timestamp;
    uint64_t md5sum;
    uint64_t revision;
  };

  virtual int load(const configure::EngineDesc& conf) = 0;

  int proc_initialize_impl(const configure::EngineDesc& conf, bool version);

  int proc_initialize(const configure::EngineDesc& conf, bool version);

  int infer(const void* in, void* out, uint32_t batch_size = -1);

  int thrd_initialize();

  int thrd_clear();

  int proc_finalize();

  int reload();

  uint64_t version() const { return _version; }
  uint32_t thread_num() const { return _infer_thread_num; }

 private:
  int parse_version_info(const configure::EngineDesc& config, bool version);

  bool check_need_reload();

  bool check_timestamp_ne();

  bool check_timestamp_gt();

  bool check_md5sum() { return false; }

  bool check_revision() { return false; }

 protected:
  // Model directory
  std::string _model_dir;

  // The description of inference engine
  configure::EngineDesc _conf;

 private:
  // Tag file of reloadable model
  std::string _reload_tag_file;

  // Type of reload, e.g. timestamp_ne, timestamp_gt, md5sum, reversion
  std::string _reload_type;

  // Record the last loading infermation
  ReloadableRecord _last_record;

  // Number of inference threads
  uint32_t _infer_thread_num;

  // Size of inference batch
  uint32_t _infer_batch_size;

  // Need to align batch_size in inferring
  bool _infer_overrun;

  // allow to split request in inferring
  bool _allow_split_request;
  // model version
  uint64_t _version;
};

// Lock free switching two models and cube caches
template <typename EngineCore>
struct ModelData {
  ModelData() : current_idx(1) {
    cores[0] = nullptr;
    cores[1] = nullptr;
    caches[0] = nullptr;
    caches[1] = nullptr;
  }

  ~ModelData() {
    delete cores[0];
    delete cores[1];
    delete caches[0];
    delete caches[1];
  }

  void* get_core() { return cores[current_idx]->get(); }

  CubeCache* get_cache() { return caches[current_idx]; }

  EngineCore* cores[2];
  CubeCache* caches[2];
  uint32_t current_idx;
};

template <typename EngineCore>
class DBReloadableInferEngine : public ReloadableInferEngine {
 public:
  virtual ~DBReloadableInferEngine() {}

  int proc_initialize(const configure::EngineDesc& conf, bool version) {
    THREAD_KEY_CREATE(&_skey, NULL);
    THREAD_MUTEX_INIT(&_mutex, NULL);
    _gpu_index = 0;
    return ReloadableInferEngine::proc_initialize(conf, version);
  }

  // 进程初始化会调用load，但由于未执行线程初始化，所以_reload_vec为空,不再继续执行。
  // 热加载的话会调用load，由于线程已经初始化，_reload_vec不为空，所以继续执行load_data操作加载数据。
  // 线程初始化会执行load_data操作加载数据，然后将engine加入_reload_vec中。
  // 每个模型只有一个CloneDBReloadableInferEngine对象。
  // 但一个CloneDBReloadableInferEngine对象，可以包含N个EngineCore。
  virtual int load(const configure::EngineDesc& conf) {
    if (_reload_vec.empty()) {
      return 0;
    }
    _gpu_index = 0;
    for (uint32_t ti = 0; ti < _reload_vec.size(); ++ti) {
      if (load_data(_reload_vec[ti], conf) != 0) {
        LOG(ERROR) << "Failed reload engine model: " << ti;
        return -1;
      }
    }

    LOG(WARNING) << "Succ load engine, path: " << conf.model_dir();
    RequestCache::GetSingleton()->Clear();
    return 0;
  }

  virtual int load_data(ModelData<EngineCore>* md,
                        const configure::EngineDesc& conf) {
    uint32_t next_idx = (md->current_idx + 1) % 2;

    // reload engine core
    if (md->cores[next_idx]) {
      delete md->cores[next_idx];
    }
    md->cores[next_idx] = new (std::nothrow) EngineCore;
    if (nullptr == md->cores[next_idx]) {
      LOG(ERROR) << "Allocating memory failed. ";
      return -1;
    }
    size_t gpu_ids_num = conf.gpu_ids_size();
    im::bsf::AutoMutex lock(_mutex);
    int gpu_id = -1;
    if (gpu_ids_num > 0) {
      gpu_id = conf.gpu_ids(_gpu_index % gpu_ids_num);
    }
    LOG(WARNING) << "Loading EngineCore[" << next_idx << "] ...";
    if (!md->cores[next_idx] ||
        md->cores[next_idx]->create(conf, gpu_id) != 0) {
      LOG(ERROR) << "Failed create model, path: " << conf.model_dir();
      return -1;
    }
    _gpu_index++;
    LOG(WARNING) << "Loading EngineCore[" << next_idx << "] done.";

    // reload cube cache
    if (nullptr == md->caches[next_idx]) {
      md->caches[next_idx] = new (std::nothrow) CubeCache;
    }

    if (nullptr == md->caches[next_idx]) {
      LOG(ERROR) << "Allocating memory failed.";
      return -1;
    }
    LOG(WARNING) << "Loading cube cache[" << next_idx << "] ...";
    std::string model_path = conf.model_dir();
    if (access(model_path.c_str(), F_OK) == 0) {
      std::string cube_cache_path = model_path + "/cube_cache";
      int reload_cache_ret = md->caches[next_idx]->reload_data(cube_cache_path);
      LOG(WARNING) << "Loading cube cache[" << next_idx << "] done.";
    } else {
      LOG(ERROR) << "model_path " << model_path
                 << " is not exits. Ignore cube cache!";
    }

    // switch current_idx
    md->current_idx = next_idx;
    LOG(WARNING)
        << "Reload model and cube cache done. switching to current_idx["
        << next_idx << "]";
    return 0;
  }

  virtual int thrd_initialize_impl() {
    ModelData<EngineCore>* md = new (std::nothrow) ModelData<EngineCore>;
    if (!md || load_data(md, _conf) != 0) {
      LOG(ERROR) << "Failed create thread data from " << _conf.model_dir();
      return -1;
    }

    THREAD_SETSPECIFIC(_skey, md);
    im::bsf::AutoMutex lock(_mutex);
    _reload_vec.push_back(md);
    return 0;
  }

  int thrd_clear_impl() {
    // actually, there are 2 kinds of multi-thread.
    // 1. brpc thread 2. bsf Task thread
    // each request is in 1-single brpc thread.
    // IF (bsf Task thread is not used)
    // every single brpc thread corresponds to all the DBReloadableInferEngines.
    // each request runs all models in 1-single brpc thread.
    // every single brpc thread will create or clone N predictor.
    // N = the number of Model.
    // so if there are 2 models, and --thread 10.
    // each brpc thread will create predictor of Model-1 and Model-2.
    // there are totally 10 predictors of Model-1 and 10 predictors of Model-2
    // cause there are 10 brpc threads.

    // IF bsf Task thread is used。
    // there will be a ThreadPool called bsf TaskExecutor.
    // TaskExecutorVector is the vector of TaskExecutor.
    // the number of TaskExecutor equals to the number of Model.
    // 1 TaskExecutor corresponding to 1 Model.
    // 1 TaskExecutor have N bsf threads.
    // 1 bsf thread corresponds to 1 predictor of
    // the Model corresponding to the TaskExecutor.
    // brpc thread only put the data into the task_queue(which is in
    // TaskExecutor)
    // EngineCore->infer() is running in bsf Task thread.

    // MempoolWrapper::instance() is actually a Thread-Local Mempool.
    // so it belongs to a single Thread.
    return 0;
  }

  int thrd_finalize_impl() { return 0; }

  int proc_finalize_impl() {
    THREAD_KEY_DELETE(_skey);
    THREAD_MUTEX_DESTROY(&_mutex);
    return 0;
  }

  EngineCore* get_core() {
    ModelData<EngineCore>* md =
        (ModelData<EngineCore>*)THREAD_GETSPECIFIC(_skey);
    if (!md) {
      LOG(ERROR) << "Failed get thread specific data";
      return NULL;
    }
    return md->cores[md->current_idx];
  }

  CubeCache* get_cube_cache() {
    ModelData<EngineCore>* md =
        (ModelData<EngineCore>*)THREAD_GETSPECIFIC(_skey);
    if (!md) {
      LOG(ERROR) << "Failed get thread specific data";
      return NULL;
    }
    return md->get_cache();
  }

 protected:
  THREAD_KEY_T _skey;
  THREAD_MUTEX_T _mutex;

  // vector of all model engines
  std::vector<ModelData<EngineCore>*> _reload_vec;

  // gpu card id
  int _gpu_index = 0;
};

// 多个EngineCore共用同一份模型数据
template <typename EngineCore>
class CloneDBReloadableInferEngine
    : public DBReloadableInferEngine<EngineCore> {
 public:
  virtual ~CloneDBReloadableInferEngine() {}

  // 进程初始化会调用load，但由于未执行线程初始化，所以_reload_vec为空,不再继续执行。
  // 热加载的话会调用load，由于线程已经初始化，_reload_vec不为空，所以继续执行load_data操作加载数据。
  // 线程初始化会执行load_data操作加载数据，然后将engine加入_reload_vec中。
  // 每个模型只有一个CloneDBReloadableInferEngine对象。
  // 但一个CloneDBReloadableInferEngine对象，可以包含N个EngineCore。

  virtual int load_data(ModelData<EngineCore>* md,
                        const configure::EngineDesc& conf) {
    int tid = syscall(SYS_gettid);
    uint32_t next_idx = (md->current_idx + 1) % 2;
    if (md->cores[next_idx]) {
      delete md->cores[next_idx];
    }
    md->cores[next_idx] = new (std::nothrow) EngineCore;

    if (nullptr == md->caches[next_idx]) {
      md->caches[next_idx] = new (std::nothrow) CubeCache;
    }
    if (nullptr == md->cores[next_idx] || nullptr == md->caches[next_idx]) {
      LOG(ERROR) << "Allocating memory fail.";
      return -1;
    }
    // params.dump();
    // gpu_ids_num > 0 is always true.
    // if use CPU, gpu_ids = [-1].
    // if gpu_ids_num = 0, which means no gpuid is given.
    // so we should set gpu_ids_num = 1, and gpu_id = -1.
    // so that we can create at least 1 predictor.
    size_t gpu_ids_num = conf.gpu_ids_size();
    im::bsf::AutoMutex lock(DBReloadableInferEngine<EngineCore>::_mutex);
    int gpu_id = -1;
    if (gpu_ids_num > 0) {
      gpu_id = conf.gpu_ids(DBReloadableInferEngine<EngineCore>::_gpu_index %
                            gpu_ids_num);
    } else {
      gpu_ids_num = 1;
    }

    // _gpu_index will be set to be 0, when load() or proc_initial() is called.
    // _gpu_index < gpu_ids_num, means there are predictors still not create
    // on some GPU card.
    // so we need to create the predictor.
    // _gpu_index >= gpu_ids_num, means each GPU card has already create one.
    // so we need to clone the predictor.
    LOG(WARNING) << "tid:" << tid << " Loading clone model ...";
    if (DBReloadableInferEngine<EngineCore>::_gpu_index < gpu_ids_num) {
      // create cores
      if (md->cores[next_idx]->create(conf, gpu_id) != 0) {
        LOG(ERROR) << "Failed create model, path: " << conf.model_dir();
        return -1;
      }
      // create caches
      std::string model_path = conf.model_dir();
      if (access(model_path.c_str(), F_OK) == 0) {
        std::string cube_cache_path = model_path + "/cube_cache";
        int reload_cache_ret =
            md->caches[next_idx]->reload_data(cube_cache_path);
        LOG(WARNING) << "create cube cache[" << next_idx << "] done.";
      } else {
        LOG(WARNING) << "model_path " << model_path
                     << " is not exits. Ignore cube cache!";
      }

      DBReloadableInferEngine<EngineCore>::_gpu_index++;
      // md->current_idx = next_idx;
      if (_cloneTemplate.size() <
          DBReloadableInferEngine<EngineCore>::_gpu_index) {
        _cloneTemplate.push_back(md);
      } else {
        _cloneTemplate[DBReloadableInferEngine<EngineCore>::_gpu_index - 1] =
            md;
      }
    } else {
      int template_index = DBReloadableInferEngine<EngineCore>::_gpu_index %
                           _cloneTemplate.size();

      // clone cores
      if (md->cores[next_idx]->clone(
              _cloneTemplate[template_index]->get_core()) != 0) {
        LOG(ERROR) << "Failed clone model from core";
        return -1;
      }
      // clone caches
      md->caches[next_idx] = _cloneTemplate[template_index]->get_cache();
      LOG(WARNING) << "tid:" << tid << " clone caches done";

      DBReloadableInferEngine<EngineCore>::_gpu_index++;
    }

    // switch current_idx
    md->current_idx = next_idx;
    LOG(WARNING)
        << "[" << tid
        << "] Reload clone model and cube cache done. switching to current_idx["
        << next_idx << "]";

    return 0;
  }

 protected:
  // 模板EngineCore，如果已创建，则多个线程级EngineCore共用该对象的模型数据
  std::vector<ModelData<EngineCore>*> _cloneTemplate;
};

template <typename EngineCore>
#ifdef WITH_TRT
class FluidInferEngine : public DBReloadableInferEngine<EngineCore> {
#else
class FluidInferEngine : public CloneDBReloadableInferEngine<EngineCore> {
#endif
 public:  // NOLINT
  FluidInferEngine() {}
  ~FluidInferEngine() {}
  typedef std::vector<paddle::PaddleTensor> TensorVector;
  int infer_impl(const void* in, void* out, uint32_t batch_size = -1) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long start = tv.tv_sec * 1000000 + tv.tv_usec;
    // First of all, get the real core acording to the
    // Template parameter <EngineCore>.
    EngineCore* core = DBReloadableInferEngine<EngineCore>::get_core();
    if (!core || !core->get()) {
      LOG(ERROR) << "Failed get fluid core in infer_impl()";
      return -1;
    }
    // We use the for loop to process the input data.
    // Inside each for loop, use the in[i]->name as inputName and call
    // 'core->GetInputHandle(inputName)' to get the pointer of InputData.
    // Set the lod and shape information of InputData first.
    // Then copy data from cpu to the core.
    const TensorVector* tensorVector_in_pointer =
        reinterpret_cast<const TensorVector*>(in);
    for (int i = 0; i < tensorVector_in_pointer->size(); ++i) {
      auto lod_tensor_in =
          core->GetInputHandle((*tensorVector_in_pointer)[i].name);
      lod_tensor_in->SetLoD((*tensorVector_in_pointer)[i].lod);
      lod_tensor_in->Reshape((*tensorVector_in_pointer)[i].shape);
      void* origin_data = (*tensorVector_in_pointer)[i].data.data();
      // Because the core needs to determine the size of memory space
      // according to the data type passed in.
      // The pointer type of data must be one of
      // float *,int64_t*,int32_t* instead void*.
      if ((*tensorVector_in_pointer)[i].dtype == paddle::PaddleDType::FLOAT32) {
        float* data = static_cast<float*>(origin_data);
        lod_tensor_in->CopyFromCpu(data);
      } else if ((*tensorVector_in_pointer)[i].dtype ==
                 paddle::PaddleDType::INT64) {
        int64_t* data = static_cast<int64_t*>(origin_data);
        lod_tensor_in->CopyFromCpu(data);
      } else if ((*tensorVector_in_pointer)[i].dtype ==
                 paddle::PaddleDType::INT32) {
        int32_t* data = static_cast<int32_t*>(origin_data);
        lod_tensor_in->CopyFromCpu(data);
      } else if ((*tensorVector_in_pointer)[i].dtype ==
                 paddle::PaddleDType::UINT8) {
        uint8_t* data = static_cast<uint8_t*>(origin_data);
        lod_tensor_in->CopyFromCpu(data);
      } else if ((*tensorVector_in_pointer)[i].dtype ==
                 paddle::PaddleDType::INT8) {
        int8_t* data = static_cast<int8_t*>(origin_data);
        lod_tensor_in->CopyFromCpu(data);
      } else if ((*tensorVector_in_pointer)[i].dtype ==
                 paddle::PaddleDType::FLOAT16) {
        phi::dtype::float16* data =
            static_cast<phi::dtype::float16*>(origin_data);
        lod_tensor_in->CopyFromCpu(data);
      } else {
        LOG(ERROR) << "Inference not support type["
                   << (*tensorVector_in_pointer)[i].dtype << "],name["
                   << (*tensorVector_in_pointer)[i].name << "]"
                   << " copy into core failed!";
      }
      VLOG(2) << "Tensor:name=" << (*tensorVector_in_pointer)[i].name
              << ";in_dtype=" << (*tensorVector_in_pointer)[i].dtype
              << ";tensor_dtype=" << lod_tensor_in->type();
    }
    // After the input data is passed in,
    // call 'core->Run()' perform the prediction process.
    if (!core->Run()) {
      LOG(ERROR) << "Failed run fluid family core";
      return -1;
    }
    // In order to get the results,
    // first, call the 'core->GetOutputNames()' to get the name of output
    // (which is a dict like {OutputName:pointer of OutputValue}).
    // Then, use for-loop to get OutputValue by calling 'core->GetOutputHandle'.
    std::vector<std::string> outnames = core->GetOutputNames();
    std::vector<int> output_shape;
    int out_num = 0;
    int dataType = 0;
    void* databuf_data = NULL;
    char* databuf_char = NULL;
    size_t databuf_size = 0;
    TensorVector* tensorVector_out_pointer =
        reinterpret_cast<TensorVector*>(out);
    if (!tensorVector_out_pointer) {
      LOG(ERROR) << "tensorVector_out_pointer is nullptr,error";
      return -1;
    }
    // Get the type and shape information of OutputData first.
    // then copy data to cpu from the core.
    // The pointer type of data_out must be one of
    // float *,int64_t*,int32_t* instead void*.
    for (int i = 0; i < outnames.size(); ++i) {
      auto lod_tensor_out = core->GetOutputHandle(outnames[i]);
      output_shape = lod_tensor_out->shape();
      out_num = std::accumulate(
          output_shape.begin(), output_shape.end(), 1, std::multiplies<int>());
      dataType = lod_tensor_out->type();
      if (dataType == paddle::PaddleDType::FLOAT32) {
        databuf_size = out_num * sizeof(float);
        databuf_data = MempoolWrapper::instance().malloc(databuf_size);
        if (!databuf_data) {
          LOG(ERROR) << "Malloc failed, size: " << databuf_size;
          return -1;
        }
        float* data_out = reinterpret_cast<float*>(databuf_data);
        lod_tensor_out->CopyToCpu(data_out);
        databuf_char = reinterpret_cast<char*>(data_out);
      } else if (dataType == paddle::PaddleDType::INT64) {
        databuf_size = out_num * sizeof(int64_t);
        databuf_data = MempoolWrapper::instance().malloc(databuf_size);
        if (!databuf_data) {
          LOG(ERROR) << "Malloc failed, size: " << databuf_size;
          return -1;
        }
        int64_t* data_out = reinterpret_cast<int64_t*>(databuf_data);
        lod_tensor_out->CopyToCpu(data_out);
        databuf_char = reinterpret_cast<char*>(data_out);
      } else if (dataType == paddle::PaddleDType::INT32) {
        databuf_size = out_num * sizeof(int32_t);
        databuf_data = MempoolWrapper::instance().malloc(databuf_size);
        if (!databuf_data) {
          LOG(ERROR) << "Malloc failed, size: " << databuf_size;
          return -1;
        }
        int32_t* data_out = reinterpret_cast<int32_t*>(databuf_data);
        lod_tensor_out->CopyToCpu(data_out);
        databuf_char = reinterpret_cast<char*>(data_out);
      } else if (dataType == paddle::PaddleDType::UINT8) {
        databuf_size = out_num * sizeof(uint8_t);
        databuf_data = MempoolWrapper::instance().malloc(databuf_size);
        if (!databuf_data) {
          LOG(ERROR) << "Malloc failed, size: " << databuf_size;
          return -1;
        }
        uint8_t* data_out = reinterpret_cast<uint8_t*>(databuf_data);
        lod_tensor_out->CopyToCpu(data_out);
        databuf_char = reinterpret_cast<char*>(data_out);
      } else if (dataType == paddle::PaddleDType::INT8) {
        databuf_size = out_num * sizeof(int8_t);
        databuf_data = MempoolWrapper::instance().malloc(databuf_size);
        if (!databuf_data) {
          LOG(ERROR) << "Malloc failed, size: " << databuf_size;
          return -1;
        }
        int8_t* data_out = reinterpret_cast<int8_t*>(databuf_data);
        lod_tensor_out->CopyToCpu(data_out);
        databuf_char = reinterpret_cast<char*>(data_out);
      } else if (dataType == paddle::PaddleDType::FLOAT16) {
        databuf_size = out_num * sizeof(phi::dtype::float16);
        databuf_data = MempoolWrapper::instance().malloc(databuf_size);
        if (!databuf_data) {
          LOG(ERROR) << "Malloc failed, size: " << databuf_size;
          return -1;
        }
        phi::dtype::float16* data_out =
            reinterpret_cast<phi::dtype::float16*>(databuf_data);
        lod_tensor_out->CopyToCpu(data_out);
        databuf_char = reinterpret_cast<char*>(data_out);
      }

      // Because task scheduling requires OPs to use 'Channel'
      // (which is a data structure) to transfer data between OPs.
      // We need to copy the processed data to the 'Channel' for the next OP.
      // In this function, it means we should copy the 'databuf_char' to
      // 'void* out'.(which is also called ‘tensorVector_out_pointer’)
      paddle::PaddleTensor tensor_out;
      tensor_out.name = outnames[i];
      tensor_out.dtype = paddle::PaddleDType(dataType);
      tensor_out.shape.assign(output_shape.begin(), output_shape.end());
      std::vector<std::vector<size_t>> out_lod = lod_tensor_out->lod();
      for (int li = 0; li < out_lod.size(); ++li) {
        std::vector<size_t> lod_element;
        lod_element.assign(out_lod[li].begin(), out_lod[li].end());
        tensor_out.lod.push_back(lod_element);
      }
      paddle::PaddleBuf paddleBuf(databuf_char, databuf_size);
      tensor_out.data = paddleBuf;
      tensorVector_out_pointer->push_back(tensor_out);
    }
    gettimeofday(&tv, NULL);
    long end = tv.tv_sec * 1000000 + tv.tv_usec;
    long total_time = end - start;
    if (PrometheusMetric::Enabled()) {
      PrometheusMetricManager::GetGeneralSingleton()
          ->MetricInferenceCount()
          .Increment(1);
      PrometheusMetricManager::GetGeneralSingleton()
          ->MetricInferenceDuration()
          .Increment(total_time);
    }
    return 0;
  }

  int task_infer_impl(const void* in, void* out) {  // NOLINT
    return infer_impl(in, out);
  }

  CubeCache* get_cube_cache() {
    return DBReloadableInferEngine<EngineCore>::get_cube_cache();
  }
};

typedef FactoryPool<InferEngine> StaticInferFactory;

class VersionedInferEngine : public InferEngine {
 public:
  VersionedInferEngine() { _versions.clear(); }
  ~VersionedInferEngine() {}

  int proc_initialize(const configure::EngineDesc& conf);

  int proc_initialize(const configure::EngineDesc& conf, bool version);

  int proc_finalize();

  int thrd_initialize();

  int thrd_clear();

  int thrd_finalize();

  int reload();

  uint64_t version() const;

  // inference interface
  InferEngine* default_engine() const;

  int infer(const void* in, void* out, uint32_t batch_size);

  template <typename T>
  T* get_core();

  CubeCache* get_cube_cache();

  // versioned inference interface
  int infer(const void* in, void* out, uint32_t batch_size, uint64_t version);

  template <typename T>
  T* get_core(const uint64_t version);

  int proc_initialize_impl(const configure::EngineDesc& conf, bool);

  int thrd_initialize_impl();

  int thrd_finalize_impl();

  int thrd_clear_impl();

  int proc_finalize_impl();

  int infer_impl(const void* in, void* out, uint32_t batch_size = -1);

  int task_infer_impl(const void* in, void* out);

 private:
  boost::unordered_map<uint64_t, InferEngine*> _versions;
};

class InferManager {
 public:
  static InferManager& instance() {
    static InferManager ins;
    return ins;
  }

  int proc_initialize(const char* path,
                      const char* file,
                      std::shared_ptr<int> engine_index_ptr);

  int set_taskexecutor_num(size_t total_engine_num);

  int thrd_initialize();

  int thrd_clear();

  int reload();

  int thrd_finalize();

  int proc_finalize();

  // Inference interface
  int infer(const char* model_name,
            const void* in,
            void* out,
            uint32_t batch_size = -1);

  // get engine core
  template <typename T>
  T* get_core(const char* model_name);

  // get cube cache
  CubeCache* get_cube_cache(const char* model_name);

  // Versioned inference interface
  int infer(const char* model_name,
            const void* in,
            void* out,
            uint32_t batch_size,
            uint64_t version);

  // Versioned get engine core
  template <typename T>
  T* get_core(const char* model_name, const uint64_t version);

  // query model version
  int query_version(const std::string& model, uint64_t& version);

 private:
  boost::unordered_map<std::string, VersionedInferEngine*> _map;
};

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
