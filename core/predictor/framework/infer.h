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
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <utility>
#include <vector>
#include <numeric>
#include "core/predictor/common/inner_common.h"
#include "core/predictor/framework/bsf.h"
#include "core/predictor/framework/factory.h"
#include "core/predictor/framework/infer_data.h"
#include "paddle_inference_api.h"  // NOLINT
namespace baidu {
namespace paddle_serving {
namespace predictor {

using configure::ModelToolkitConf;

class InferEngineCreationParams {
 public:
  InferEngineCreationParams() {
    _path = "";
    _enable_memory_optimization = false;
    _enable_ir_optimization = false;
    _static_optimization = false;
    _force_update_static_cache = false;
    _use_trt = false;
    _use_lite = false;
    _use_xpu = false;
  }

  void set_path(const std::string& path) { _path = path; }

  void set_enable_memory_optimization(bool enable_memory_optimization) {
    _enable_memory_optimization = enable_memory_optimization;
  }

  void set_enable_ir_optimization(bool enable_ir_optimization) {
    _enable_ir_optimization = enable_ir_optimization;
  }

  void set_use_trt(bool use_trt) { _use_trt = use_trt; }

  void set_use_lite(bool use_lite) { _use_lite = use_lite; }

  void set_use_xpu(bool use_xpu) { _use_xpu = use_xpu; }

  bool enable_memory_optimization() const {
    return _enable_memory_optimization;
  }

  bool enable_ir_optimization() const { return _enable_ir_optimization; }

  bool use_trt() const { return _use_trt; }

  bool use_lite() const { return _use_lite; }

  bool use_xpu() const { return _use_xpu; }

  void set_static_optimization(bool static_optimization = false) {
    _static_optimization = static_optimization;
  }

  void set_force_update_static_cache(bool force_update_static_cache = false) {
    _force_update_static_cache = force_update_static_cache;
  }

  bool static_optimization() const { return _static_optimization; }

  bool force_update_static_cache() const { return _force_update_static_cache; }

  std::string get_path() const { return _path; }

  void dump() const {
    LOG(INFO) << "InferEngineCreationParams: "
              << "model_path = " << _path << ", "
              << "enable_memory_optimization = " << _enable_memory_optimization
              << ", "
              << "enable_tensorrt = " << _use_trt << ", "
              << "enable_lite = " << _use_lite << ", "
              << "enable_xpu = " << _use_xpu << ", "
              << "enable_ir_optimization = " << _enable_ir_optimization << ", "
              << "static_optimization = " << _static_optimization << ", "
              << "force_update_static_cache = " << _force_update_static_cache;
  }

 private:
  std::string _path;
  bool _enable_memory_optimization;
  bool _enable_ir_optimization;
  bool _static_optimization;
  bool _force_update_static_cache;
  bool _use_trt;
  bool _use_lite;
  bool _use_xpu;
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
  virtual int infer(const void* in, void* out, uint32_t batch_size = -1) { return infer_impl(in, out, batch_size); }

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

  union last_check_status {
    time_t last_timestamp;
    uint64_t last_md5sum;
    uint64_t last_revision;
  };

  typedef im::bsf::Task<Tensor, Tensor> TaskT;

  virtual int load(const InferEngineCreationParams& params) = 0;

  int proc_initialize_impl(const configure::EngineDesc& conf, bool version) {
    _reload_tag_file = conf.reloadable_meta();
    _reload_mode_tag = conf.reloadable_type();
    _model_data_path = conf.model_data_path();
    _infer_thread_num = conf.runtime_thread_num();
    _infer_batch_size = conf.batch_infer_size();
    _infer_batch_align = conf.enable_batch_align();

    bool enable_memory_optimization = false;
    if (conf.has_enable_memory_optimization()) {
      enable_memory_optimization = conf.enable_memory_optimization();
    }

    bool static_optimization = false;
    if (conf.has_static_optimization()) {
      static_optimization = conf.static_optimization();
    }

    bool force_update_static_cache = false;
    if (conf.has_force_update_static_cache()) {
      force_update_static_cache = conf.force_update_static_cache();
    }

    if (conf.has_enable_ir_optimization()) {
      _infer_engine_params.set_enable_ir_optimization(
          conf.enable_ir_optimization());
    }

    _infer_engine_params.set_path(_model_data_path);
    if (enable_memory_optimization) {
      _infer_engine_params.set_enable_memory_optimization(true);
      _infer_engine_params.set_static_optimization(static_optimization);
      _infer_engine_params.set_force_update_static_cache(
          force_update_static_cache);
    }

    if (conf.has_use_trt()) {
      _infer_engine_params.set_use_trt(conf.use_trt());
    }

    if (conf.has_use_lite()) {
      _infer_engine_params.set_use_lite(conf.use_lite());
    }

    if (conf.has_use_xpu()) {
      _infer_engine_params.set_use_xpu(conf.use_xpu());
    }

    if (!check_need_reload() || load(_infer_engine_params) != 0) {
      LOG(ERROR) << "Failed load model_data_path" << _model_data_path;
      return -1;
    }

    if (parse_version_info(conf, version) != 0) {
      LOG(ERROR) << "Failed parse version info";
      return -1;
    }

    LOG(WARNING) << "Succ load model_data_path" << _model_data_path;
    return 0;
  }

  int proc_initialize(const configure::EngineDesc& conf, bool version) {
    if (proc_initialize_impl(conf, version) != 0) {
      LOG(ERROR) << "Failed proc initialize impl";
      return -1;
    }

    // init bsf framework
    if (_infer_thread_num <= 0) {
      return 0;
    }

    // init bsf framework
    im::bsf::TaskExecutor<TaskT>::instance()->set_thread_init_fn(
        boost::bind(&InferEngine::thrd_initialize_impl, this));
    im::bsf::TaskExecutor<TaskT>::instance()->set_thread_reset_fn(
        boost::bind(&InferEngine::thrd_clear_impl, this));
    im::bsf::TaskExecutor<TaskT>::instance()->set_thread_callback_fn(
        boost::bind(&InferEngine::task_infer_impl, this, _1, _2));
    im::bsf::TaskExecutor<TaskT>::instance()->set_batch_size(_infer_batch_size);
    im::bsf::TaskExecutor<TaskT>::instance()->set_batch_align(
        _infer_batch_align);
    if (im::bsf::TaskExecutor<TaskT>::instance()->start(_infer_thread_num) !=
        0) {
      LOG(ERROR) << "Failed start bsf executor, threads:" << _infer_thread_num;
      return -1;
    }

    LOG(WARNING) << "Enable batch schedule framework, thread_num:"
                 << _infer_thread_num << ", batch_size:" << _infer_batch_size
                 << ", enable_batch_align:" << _infer_batch_align;
    return 0;
  }

  int infer(const void* in, void* out, uint32_t batch_size = -1) {
    if (_infer_thread_num <= 0) {
      return infer_impl(in, out, batch_size);
    }

    im::bsf::TaskManager<Tensor, Tensor> task_manager;
    task_manager.schedule(*(reinterpret_cast<const BatchTensor*>(in)),
                          *(reinterpret_cast<BatchTensor*>(out)));
    task_manager.wait();
    return 0;
  }

  int thrd_initialize() {
    if (_infer_thread_num > 0) {
      return 0;
    }

    return thrd_initialize_impl();
  }

  int thrd_clear() {
    if (_infer_thread_num > 0) {
      return 0;
    }

    return thrd_clear_impl();
  }

  int proc_finalize() {
    if (proc_finalize_impl() != 0) {
      LOG(ERROR) << "Failed proc finalize impl";
      return -1;
    }

    if (_infer_thread_num > 0) {
      im::bsf::TaskExecutor<TaskT>::instance()->stop();
    }
    return 0;
  }

  int reload() {
    if (check_need_reload()) {
      LOG(WARNING) << "begin reload model[" << _model_data_path << "].";
      return load(_infer_engine_params);
    }
    return 0;
  }

  uint64_t version() const { return _version; }

  uint32_t thread_num() const { return _infer_thread_num; }

 private:
  int parse_version_info(const configure::EngineDesc& config, bool version) {
    _version = uint64_t(-1);
    return 0;
  }

  bool check_need_reload() {
    if (_reload_mode_tag == "timestamp_ne") {
      return check_timestamp_ne();
    } else if (_reload_mode_tag == "timestamp_gt") {
      return check_timestamp_gt();
    } else if (_reload_mode_tag == "md5sum") {
      return check_md5sum();
    } else if (_reload_mode_tag == "revision") {
      return check_revision();
    } else if (_reload_mode_tag == "none") {
      return false;
    } else {
      LOG(ERROR) << "Not support check type: " << _reload_mode_tag;
      return false;
    }
  }

  bool check_timestamp_ne() {
    struct stat st;
    if (stat(_reload_tag_file.c_str(), &st) != 0) {
      LOG(ERROR) << "Failed stat config file:" << _reload_tag_file;
      return false;
    }

    if ((st.st_mode & S_IFREG) && st.st_mtime != _last_status.last_timestamp) {
      _last_status.last_timestamp = st.st_mtime;
      return true;
    }

    return false;
  }

  bool check_timestamp_gt() {
    struct stat st;
    if (stat(_reload_tag_file.c_str(), &st) != 0) {
      LOG(ERROR) << "Failed stat config file:" << _reload_tag_file;
      return false;
    }

    if ((st.st_mode & S_IFREG) && st.st_mtime > _last_status.last_timestamp) {
      _last_status.last_timestamp = st.st_mtime;
      return true;
    }

    return false;
  }

  bool check_md5sum() { return false; }

  bool check_revision() { return false; }

 protected:
  std::string _model_data_path;
  InferEngineCreationParams _infer_engine_params;

 private:
  std::string _reload_tag_file;
  std::string _reload_mode_tag;
  last_check_status _last_status;
  uint32_t _infer_thread_num;
  uint32_t _infer_batch_size;
  bool _infer_batch_align;
  uint64_t _version;
};

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

  virtual int load(const InferEngineCreationParams& params) {
    if (_reload_vec.empty()) {
      return 0;
    }

    for (uint32_t ti = 0; ti < _reload_vec.size(); ++ti) {
      if (load_data(_reload_vec[ti], params) != 0) {
        LOG(ERROR) << "Failed reload engine model: " << ti;
        return -1;
      }
    }

    LOG(WARNING) << "Succ load engine, path: " << params.get_path();

    return 0;
  }

  int load_data(ModelData<EngineCore>* md,
                const InferEngineCreationParams& params) {
    uint32_t next_idx = (md->current_idx + 1) % 2;
    if (md->cores[next_idx]) {
      delete md->cores[next_idx];
    }

    md->cores[next_idx] = new (std::nothrow) EngineCore;

    params.dump();
    if (!md->cores[next_idx] || md->cores[next_idx]->create(params) != 0) {
      LOG(ERROR) << "Failed create model, path: " << params.get_path();
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
    if (!md || load_data(md, _infer_engine_params) != 0) {
      LOG(ERROR) << "Failed create thread data from "
                 << _infer_engine_params.get_path();
      return -1;
    }

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

 private:
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

  virtual int load(const InferEngineCreationParams& params) {
    // 加载进程级模型数据
    if (!_pd ||
        DBReloadableInferEngine<EngineCore>::load_data(_pd, params) != 0) {
      LOG(ERROR) << "Failed to create common model from [" << params.get_path()
                 << "].";
      return -1;
    }
    LOG(WARNING) << "Succ load common model[" << _pd->cores[_pd->current_idx]
                 << "], path[" << params.get_path() << "].";

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

    LOG(WARNING) << "Succ load clone model, path[" << params.get_path() << "]";

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

template <typename FluidFamilyCore>
#ifdef WITH_TRT
class FluidInferEngine : public DBReloadableInferEngine<FluidFamilyCore> {
#else
class FluidInferEngine : public CloneDBReloadableInferEngine<FluidFamilyCore> {
#endif
 public:  // NOLINT
  FluidInferEngine() {}
  ~FluidInferEngine() {}
  typedef std::vector<paddle::PaddleTensor> TensorVector;
  int infer_impl(const void* in, void* out, uint32_t batch_size = -1) {
    //First of all, get the real core acording to the template parameter 'FluidFamilyCore'.
    FluidFamilyCore* core =DBReloadableInferEngine<FluidFamilyCore>::get_core();
    if (!core || !core->get()) {
      LOG(ERROR) << "Failed get fluid core in infer_impl()";
      return -1;
    }
    //We use the for loop to process the input data.
    //Inside each for loop, use the in[i]->name as inputName and call 'core->GetInputHandle(inputName)' to get the pointer of InputData.
    //Set the lod and shape information of InputData first. then copy data from cpu to the core.
    const TensorVector* tensorVector_in_pointer = reinterpret_cast<const TensorVector*>(in);
    for(int i =0; i< tensorVector_in_pointer->size();++i){
      auto lod_tensor_in = core->GetInputHandle((*tensorVector_in_pointer)[i].name);
      lod_tensor_in->SetLoD((*tensorVector_in_pointer)[i].lod);
      lod_tensor_in->Reshape((*tensorVector_in_pointer)[i].shape);
      void* origin_data = (*tensorVector_in_pointer)[i].data.data();
      //Because the core needs to determine the size of memory space according to the data type passed in.
      //The pointer type of data must be one of float *,int64_t*,int32_t* instead void*.
      if ((*tensorVector_in_pointer)[i].dtype == paddle::PaddleDType::FLOAT32) {
        float* data = static_cast<float*>(origin_data);
        lod_tensor_in->CopyFromCpu(data);
      }else if ((*tensorVector_in_pointer)[i].dtype == paddle::PaddleDType::INT64) {
        int64_t* data = static_cast<int64_t*>(origin_data);
        lod_tensor_in->CopyFromCpu(data);
      }else if ((*tensorVector_in_pointer)[i].dtype == paddle::PaddleDType::INT32) {
        int32_t* data = static_cast<int32_t*>(origin_data);
        lod_tensor_in->CopyFromCpu(data);
      }
    }
    //After the input data is passed in, call 'core->Run()' perform the prediction process.
    if (!core->Run()) {
        LOG(ERROR) << "Failed run fluid family core";
        return -1;
    }
    
    //In order to get the results, first, call the 'core->GetOutputNames()' to get the name of output(which is a dict like {OutputName:pointer of OutputValue}).
    //Then, use for-loop to get OutputValue by calling 'core->GetOutputHandle'.
    std::vector<std::string> outnames = core->GetOutputNames();
    std::vector<int> output_shape;
    int out_num =0;
    int dataType =0;
    void* databuf_data = NULL;
    char* databuf_char = NULL;
    size_t databuf_size = 0;
    TensorVector* tensorVector_out_pointer = reinterpret_cast<TensorVector*>(out);
    if (!tensorVector_out_pointer) {
      LOG(ERROR) << "tensorVector_out_pointer is nullptr,error";
      return -1;
    }
    //Get the type and shape information of OutputData first. then copy data to cpu from the core.
    //The pointer type of data_out must be one of float *,int64_t*,int32_t* instead void*.
    for (int i = 0; i < outnames.size(); ++i){
      auto lod_tensor_out = core->GetOutputHandle(outnames[i]);
      output_shape = lod_tensor_out->shape();
      out_num = std::accumulate(output_shape.begin(), output_shape.end(), 1, std::multiplies<int>());
      dataType = lod_tensor_out->type();
      if (dataType == paddle::PaddleDType::FLOAT32) {
        databuf_size = out_num*sizeof(float);
        databuf_data = MempoolWrapper::instance().malloc(databuf_size);
        if (!databuf_data) {
            LOG(ERROR) << "Malloc failed, size: " << databuf_size;
            return -1;
        }
        float* data_out = reinterpret_cast<float*>(databuf_data);
        lod_tensor_out->CopyToCpu(data_out);
        databuf_char = reinterpret_cast<char*>(data_out);
      }else if (dataType == paddle::PaddleDType::INT64) {
        databuf_size = out_num*sizeof(int64_t);
        databuf_data = MempoolWrapper::instance().malloc(databuf_size);
        if (!databuf_data) {
            LOG(ERROR) << "Malloc failed, size: " << databuf_size;
            return -1;
        }
        int64_t* data_out = reinterpret_cast<int64_t*>(databuf_data);
        lod_tensor_out->CopyToCpu(data_out);
        databuf_char = reinterpret_cast<char*>(data_out);
      }else if (dataType == paddle::PaddleDType::INT32) {
        databuf_size = out_num*sizeof(int32_t);
        databuf_data = MempoolWrapper::instance().malloc(databuf_size);
        if (!databuf_data) {
            LOG(ERROR) << "Malloc failed, size: " << databuf_size;
            return -1;
        }
        int32_t* data_out = reinterpret_cast<int32_t*>(databuf_data);
        lod_tensor_out->CopyToCpu(data_out);
        databuf_char = reinterpret_cast<char*>(data_out);
      }
      //Because task scheduling requires OPs to use 'Channel'(which is a data structure) to transfer data between OPs.
      //We need to copy the processed data to the 'Channel' for the next OP.
      //In this function, it means we should copy the 'databuf_char' to the pointer 'void* out'.(which is also called ‘tensorVector_out_pointer’)
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

  int proc_initialize(const configure::EngineDesc& conf) {
    if (proc_initialize(conf, false) != 0) {
      LOG(ERROR) << "Failed proc intialize engine: " << conf.name().c_str();
      return -1;
    }

    LOG(WARNING) << "Succ proc initialize engine: " << conf.name().c_str();
    return 0;
  }

  int proc_initialize(const configure::EngineDesc& conf, bool version) {
    std::string engine_type = conf.type();
    InferEngine* engine =
        StaticInferFactory::instance().generate_object(engine_type);
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
    LOG(WARNING) << "Succ proc initialize version engine: "
                 << engine->version();
    return 0;
  }

  int proc_finalize() {
    for (auto iter = _versions.begin(); iter != _versions.end(); ++iter) {
      if (iter->second->proc_finalize() != 0) {
        LOG(ERROR) << "Failed proc finalize version engine: " << iter->first;
      }
      LOG(WARNING) << "Succ proc finalize version engine: " << iter->first;
    }
    return 0;
  }

  int thrd_initialize() {
    for (auto iter = _versions.begin(); iter != _versions.end(); ++iter) {
      if (iter->second->thrd_initialize() != 0) {
        LOG(ERROR) << "Failed thrd initialize version engine: " << iter->first;
        return -1;
      }
      LOG(WARNING) << "Succ thrd initialize version engine: " << iter->first;
    }
    return 0;
  }

  int thrd_clear() {
    for (auto iter = _versions.begin(); iter != _versions.end(); ++iter) {
      if (iter->second->thrd_clear() != 0) {
        LOG(ERROR) << "Failed thrd clear version engine: " << iter->first;
        return -1;
      }
    }
    return 0;
  }

  int thrd_finalize() {
    for (auto iter = _versions.begin(); iter != _versions.end(); ++iter) {
      if (iter->second->thrd_finalize() != 0) {
        LOG(ERROR) << "Failed thrd finalize version engine: " << iter->first;
        return -1;
      }
      LOG(WARNING) << "Succ thrd finalize version engine: " << iter->first;
    }
    return 0;
  }

  int reload() {
    for (auto iter = _versions.begin(); iter != _versions.end(); ++iter) {
      if (iter->second->reload() != 0) {
        LOG(ERROR) << "Failed reload version engine: " << iter->first;
        return -1;
      }
      LOG(WARNING) << "Succ reload version engine: " << iter->first;
    }
    return 0;
  }

  uint64_t version() const {
    InferEngine* engine = default_engine();
    if (engine) {
      return engine->version();
    } else {
      return uint64_t(-1);
    }
  }

  // inference interface
  InferEngine* default_engine() const {
    if (_versions.size() != 1) {
      LOG(ERROR) << "Ambiguous default engine version:" << _versions.size();
      return NULL;
    }

    return _versions.begin()->second;
  }

  int infer(const void* in, void* out, uint32_t batch_size) {
    InferEngine* engine = default_engine();
    if (!engine) {
      LOG(WARNING) << "fail to get default engine";
      return -1;
    }
    return engine->infer(in, out, batch_size);
  }

  template <typename T>
  T* get_core() {
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

  // versioned inference interface
  int infer(const void* in, void* out, uint32_t batch_size, uint64_t version) {
    auto iter = _versions.find(version);
    if (iter == _versions.end()) {
      LOG(ERROR) << "Not found version engine: " << version;
      return -1;
    }

    return iter->second->infer(in, out, batch_size);
  }

  template <typename T>
  T* get_core(uint64_t version) {
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

  // --
  int proc_initialize_impl(const configure::EngineDesc& conf, bool) {
    return -1;
  }
  int thrd_initialize_impl() { return -1; }
  int thrd_finalize_impl() { return -1; }
  int thrd_clear_impl() { return -1; }
  int proc_finalize_impl() { return -1; }
  int infer_impl(const void* in, void* out, uint32_t batch_size = -1) { return -1; }
  int task_infer_impl(const BatchTensor& in, BatchTensor& out) {  // NOLINT
    return -1;
  }  // NOLINT

 private:
  boost::unordered_map<uint64_t, InferEngine*> _versions;
};

class InferManager {
 public:
  static InferManager& instance() {
    static InferManager ins;
    return ins;
  }

  int proc_initialize(const char* path, const char* file) {
    ModelToolkitConf model_toolkit_conf;
    if (configure::read_proto_conf(path, file, &model_toolkit_conf) != 0) {
      LOG(ERROR) << "failed load infer config, path: " << path << "/" << file;
      return -1;
    }
    size_t engine_num = model_toolkit_conf.engines_size();
    for (size_t ei = 0; ei < engine_num; ++ei) {
      LOG(INFO) << "model_toolkit_conf.engines(" << ei
                << ").name: " << model_toolkit_conf.engines(ei).name();
      std::string engine_name = model_toolkit_conf.engines(ei).name();
      VersionedInferEngine* engine = new (std::nothrow) VersionedInferEngine();
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

  int thrd_initialize() {
    for (auto it = _map.begin(); it != _map.end(); ++it) {
      if (it->second->thrd_initialize() != 0) {
        LOG(ERROR) << "Failed thrd initialize engine, name: " << it->first;
        return -1;
      }
      LOG(WARNING) << "Succ thrd initialize engine, name: " << it->first;
    }
    return 0;
  }

  int thrd_clear() {
    for (auto it = _map.begin(); it != _map.end(); ++it) {
      if (it->second->thrd_clear() != 0) {
        LOG(ERROR) << "Failed thrd clear engine, name: " << it->first;
        return -1;
      }
    }
    return 0;
  }

  int reload() {
    for (auto it = _map.begin(); it != _map.end(); ++it) {
      if (it->second->reload() != 0) {
        LOG(ERROR) << "Failed reload engine, name: " << it->first;
        return -1;
      }
    }
    return 0;
  }

  int thrd_finalize() {
    for (auto it = _map.begin(); it != _map.end(); ++it) {
      if (it->second->thrd_finalize() != 0) {
        LOG(ERROR) << "Failed thrd finalize engine, name: " << it->first;
        return -1;
      }
      LOG(WARNING) << "Succ thrd finalize engine, name: " << it->first;
    }
    return 0;
  }

  int proc_finalize() {
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
  int infer(const char* model_name,
            const void* in,
            void* out,
            uint32_t batch_size = -1) {
    auto it = _map.find(model_name);
    if (it == _map.end()) {
      LOG(WARNING) << "Cannot find engine in map, model name:" << model_name;
      return -1;
    }
    return it->second->infer(in, out, batch_size);
  }

  template <typename T>
  T* get_core(const char* model_name) {
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

  // Versioned inference interface
  int infer(const char* model_name, 
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
  T* get_core(const char* model_name, uint64_t version) {
    auto it = _map.find(model_name);
    if (it == _map.end()) {
      LOG(WARNING) << "Cannot find engine in map, model name:" << model_name;
      return NULL;
    }
    return it->second->get_core<T>(version);
  }

  int query_version(const std::string& model, uint64_t& version) {  // NOLINT
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

 private:
  boost::unordered_map<std::string, VersionedInferEngine*> _map;
};

}  // namespace predictor
}  // namespace paddle_serving
}  // namespace baidu
