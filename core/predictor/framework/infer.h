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
#include <sys/types.h>
#include <unistd.h>
#include <numeric>
#include <string>
#include <utility>
#include <vector>
#include "core/predictor/common/inner_common.h"
#include "core/predictor/framework/bsf.h"
#include "core/predictor/framework/factory.h"
#include "core/predictor/framework/infer_data.h"
#include "paddle_inference_api.h"  // NOLINT
namespace baidu {
namespace paddle_serving {
namespace predictor {

using configure::ModelToolkitConf;

class AutoLock {
 public:
  explicit AutoLock(pthread_mutex_t& mutex) : _mut(mutex) {
    pthread_mutex_lock(&mutex);
  }
  ~AutoLock() { pthread_mutex_unlock(&_mut); }

 private:
  pthread_mutex_t& _mut;
};

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
  virtual int task_infer_impl(const BatchTensor& in,
                              BatchTensor& out) = 0;  // NOLINT

  // end: framework inner call
};

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
  typedef im::bsf::Task<Tensor, Tensor> TaskT;

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
  bool _infer_batch_align;

  // model version
  uint64_t _version;
};

// Lock free switching two models
template <typename EngineCore>
struct ModelData {
  ModelData() : current_idx(1) {
    cores[0] = NULL;
    cores[1] = NULL;
  }

  ~ModelData() {
    delete cores[0];
    delete cores[1];
  }

  EngineCore* cores[2];
  uint32_t current_idx;
};

template <typename EngineCore>
class DBReloadableInferEngine : public ReloadableInferEngine {
 public:
  virtual ~DBReloadableInferEngine() {}

  int proc_initialize(const configure::EngineDesc& conf, bool version) {
    THREAD_KEY_CREATE(&_skey, NULL);
    THREAD_MUTEX_INIT(&_mutex, NULL);
    return ReloadableInferEngine::proc_initialize(conf, version);
  }

  virtual int load(const configure::EngineDesc& conf) {
    if (_reload_vec.empty()) {
      return 0;
    }

    for (uint32_t ti = 0; ti < _reload_vec.size(); ++ti) {
      if (load_data(_reload_vec[ti], conf) != 0) {
        LOG(ERROR) << "Failed reload engine model: " << ti;
        return -1;
      }
    }

    LOG(WARNING) << "Succ load engine, path: " << conf.model_dir();
    return 0;
  }

  int load_data(ModelData<EngineCore>* md, const configure::EngineDesc& conf) {
    uint32_t next_idx = (md->current_idx + 1) % 2;
    if (md->cores[next_idx]) {
      delete md->cores[next_idx];
    }

    md->cores[next_idx] = new (std::nothrow) EngineCore;

    // params.dump();
    if (!md->cores[next_idx] || md->cores[next_idx]->create(conf) != 0) {
      LOG(ERROR) << "Failed create model, path: " << conf.model_dir();
      return -1;
    }
    md->current_idx = next_idx;
    return 0;
  }

  virtual int thrd_initialize_impl() {
    // memory pool to be inited in non-serving-threads
    if (MempoolWrapper::instance().thread_initialize() != 0) {
      LOG(ERROR) << "Failed thread initialize mempool";
      return -1;
    }

    ModelData<EngineCore>* md = new (std::nothrow) ModelData<EngineCore>;
    if (!md || load_data(md, _conf) != 0) {
      LOG(ERROR) << "Failed create thread data from " << _conf.model_dir();
      return -1;
    }

    LOG(ERROR) << "THREAD_SETSPECIFIC _skey = md";
    THREAD_SETSPECIFIC(_skey, md);
    im::bsf::AutoMutex lock(_mutex);
    _reload_vec.push_back(md);
    return 0;
  }

  int thrd_clear_impl() {
    // for non-serving-threads
    if (MempoolWrapper::instance().thread_clear() != 0) {
      LOG(ERROR) << "Failed thread clear mempool";
      return -1;
    }
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

 protected:
  THREAD_KEY_T _skey;
  THREAD_MUTEX_T _mutex;
  std::vector<ModelData<EngineCore>*> _reload_vec;
};

// 多个EngineCore共用同一份模型数据
template <typename EngineCore>
class CloneDBReloadableInferEngine
    : public DBReloadableInferEngine<EngineCore> {
 public:
  virtual ~CloneDBReloadableInferEngine() {}

  virtual int proc_initialize(const configure::EngineDesc& conf, bool version) {
    _pd = new (std::nothrow) ModelData<EngineCore>;
    if (!_pd) {
      LOG(ERROR) << "Failed to allocate for ProcData";
      return -1;
    }
    return DBReloadableInferEngine<EngineCore>::proc_initialize(conf, version);
  }

  virtual int load(const configure::EngineDesc& conf) {
    // 加载进程级模型数据
    if (!_pd ||
        DBReloadableInferEngine<EngineCore>::load_data(_pd, conf) != 0) {
      LOG(ERROR) << "Failed to create common model from [" << conf.model_dir()
                 << "].";
      return -1;
    }
    LOG(WARNING) << "Succ load common model[" << _pd->cores[_pd->current_idx]
                 << "], path[" << conf.model_dir() << "].";

    if (DBReloadableInferEngine<EngineCore>::_reload_vec.empty()) {
      return 0;
    }

    for (uint32_t ti = 0;
         ti < DBReloadableInferEngine<EngineCore>::_reload_vec.size();
         ++ti) {
      if (load_data(DBReloadableInferEngine<EngineCore>::_reload_vec[ti],
                    _pd->cores[_pd->current_idx]) != 0) {
        LOG(ERROR) << "Failed reload engine model: " << ti;
        return -1;
      }
    }

    LOG(WARNING) << "Succ load clone model, path[" << conf.model_dir() << "]";
    return 0;
  }

  // 加载线程级对象，多个线程级对象共用pd_core的模型数据
  int load_data(ModelData<EngineCore>* td, EngineCore* pd_core) {
    uint32_t next_idx = (td->current_idx + 1) % 2;
    if (td->cores[next_idx]) {
      delete td->cores[next_idx];
    }

    td->cores[next_idx] = new (std::nothrow) EngineCore;
    if (!td->cores[next_idx] ||
        td->cores[next_idx]->clone(pd_core->get()) != 0) {
      LOG(ERROR) << "Failed clone model from pd_core[ " << pd_core << "], idx["
                 << next_idx << "]";
      return -1;
    }
    td->current_idx = next_idx;
    LOG(WARNING) << "td_core[" << td->cores[td->current_idx]
                 << "] clone model from pd_core[" << pd_core
                 << "] succ, cur_idx[" << td->current_idx << "].";
    return 0;
  }

  virtual int thrd_initialize_impl() {
    // memory pool to be inited in non-serving-threads
    if (MempoolWrapper::instance().thread_initialize() != 0) {
      LOG(ERROR) << "Failed thread initialize mempool";
      return -1;
    }

    ModelData<EngineCore>* md = new (std::nothrow) ModelData<EngineCore>;
    if (!md || load_data(md, _pd->cores[_pd->current_idx]) != 0) {
      LOG(ERROR) << "Failed clone thread data, origin_core["
                 << _pd->cores[_pd->current_idx] << "].";
      return -1;
    }

    THREAD_SETSPECIFIC(DBReloadableInferEngine<EngineCore>::_skey, md);
    im::bsf::AutoMutex lock(DBReloadableInferEngine<EngineCore>::_mutex);
    DBReloadableInferEngine<EngineCore>::_reload_vec.push_back(md);
    return 0;
  }

 protected:
  ModelData<EngineCore>*
      _pd;  // 进程级EngineCore，多个线程级EngineCore共用该对象的模型数据
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
      }
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
    return 0;
  }

  int task_infer_impl(const BatchTensor& in, BatchTensor& out) {  // NOLINT
    return infer_impl(&in, &out);
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

  // versioned inference interface
  int infer(const void* in, void* out, uint32_t batch_size, uint64_t version);

  template <typename T>
  T* get_core(uint64_t version);

  int proc_initialize_impl(const configure::EngineDesc& conf, bool);

  int thrd_initialize_impl();

  int thrd_finalize_impl();

  int thrd_clear_impl();

  int proc_finalize_impl();

  int infer_impl(const void* in, void* out, uint32_t batch_size = -1);

  int task_infer_impl(const BatchTensor& in, BatchTensor& out);

 private:
  boost::unordered_map<uint64_t, InferEngine*> _versions;
};

class InferManager {
 public:
  static InferManager& instance() {
    static InferManager ins;
    return ins;
  }

  int proc_initialize(const char* path, const char* file);

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

  template <typename T>
  T* get_core(const char* model_name);

  // Versioned inference interface
  int infer(const char* model_name,
            const void* in,
            void* out,
            uint32_t batch_size,
            uint64_t version);

  template <typename T>
  T* get_core(const char* model_name, uint64_t version);

  int query_version(const std::string& model, uint64_t& version);

 private:
  boost::unordered_map<std::string, VersionedInferEngine*> _map;
};

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
