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

#include <errno.h>
#include <algorithm>
#include <list>
#include <vector>

#ifdef BCLOUD
#include "base/atomicops.h"
#else
#include "butil/atomicops.h"
#endif

#include "core/predictor/common/inner_common.h"

#include "boost/function.hpp"

#include "core/predictor/framework/memory.h"
#include "paddle_inference_api.h"

namespace im {
namespace bsf {

static const size_t DEFAULT_BATCH_SIZE = 100;

// InItemT is paddle::PaddleTensor
// InVectorT std::vector<paddle::PaddleTensor>
// InVectorT means different feedvar, but not batch.
// Batch is already inside the  paddle::PaddleTensor.

// size_t `rem` records how many batch have not been put in BatchTasks.
// `rem` don`t need to be atomic, cause the operation `put` is synchronous.
// actually, the reason is that lock have been added outside the operation
// `put`.

// size_t `index` records how many batch have been processing completed.
// `index` need to be atomic, cause the operation 'notify' is asynchronous.
template <typename InItemT, typename OutItemT>
struct Task {
  typedef std::vector<InItemT> InVectorT;
  typedef std::vector<OutItemT> OutVectorT;
  typedef InItemT InType;
  typedef OutItemT OutType;
  typedef Task<InItemT, OutItemT> TaskT;
  typedef std::vector<int> ShapeVector;
  typedef std::vector<ShapeVector> VectorOfShapeVector;

  int read_fd;
  int write_fd;
  pid_t owner_tid;
  const InVectorT* inVectorT_ptr;
  OutVectorT* outVectorT_ptr;
  size_t rem;
  butil::atomic<size_t> index;

  Task() {
    read_fd = -1;
    write_fd = -1;
    owner_tid = -1;
    inVectorT_ptr = NULL;
    outVectorT_ptr = NULL;
    rem = -1;
    index.store(0, butil::memory_order_relaxed);
  }

  bool check_feedvar_valid(int feedvar_index) {
    if (feedvar_index < 0 || inVectorT_ptr->size() <= feedvar_index) {
      LOG(ERROR) << "feedvar doesnt exsit or feedvar_index error";
      return 0;
    }

    if ((*inVectorT_ptr)[feedvar_index].shape.size() <= 0) {
      LOG(ERROR) << "feedvar[" << feedvar_index << "].shape.size()<=0,error";
      return 0;
    }

    return 1;
  }

  // Now, it simply assume that the first dimension of data is batch.
  // so the batch is PaddleTensor.shape[0]

  // If batch information is added into feedvar.prototxt.
  // we can get the information from the feedvar.prototxt instead of assume.
  size_t feedvar_batch_size(int feedvar_index) {
    if (!check_feedvar_valid(feedvar_index)) {
      return 0;
    }

    return (*inVectorT_ptr)[feedvar_index].shape[0];
  }

  size_t feedvar_element_bytesize(int feedvar_index) {
    if (!check_feedvar_valid(feedvar_index)) {
      return 0;
    }
    int dtype = (*inVectorT_ptr)[feedvar_index].dtype;
    if (dtype == paddle::PaddleDType::INT64) {
      return sizeof(int64_t);
    }
    if (dtype == paddle::PaddleDType::FLOAT32) {
      return sizeof(float);
    }
    if (dtype == paddle::PaddleDType::INT32) {
      return sizeof(int32_t);
    }
    if (dtype == paddle::PaddleDType::UINT8) {
      return sizeof(char);
    }
    return 0;
  }

  // Now, the implementation of this function is based on assumption
  // that shape [0] = batch_size.
  size_t feedvar_element_num(int feedvar_index) {
    if (!check_feedvar_valid(feedvar_index)) {
      return 0;
    }
    size_t element_num = 1;
    if ((*inVectorT_ptr)[feedvar_index].shape.size() == 1) {
      // cause shape[0] is batch_size.
      // [10,1] = [10], so if shape[1] doesn`t exist.
      // should return 1.
      return 1;
    }
    // start from shape[1], cause shape[0] = batch_size.
    for (int i = 1; i < (*inVectorT_ptr)[feedvar_index].shape.size(); ++i) {
      element_num *= (*inVectorT_ptr)[feedvar_index].shape[i];
    }
    return element_num;
  }

  size_t feedvar_bytesize(int feedvar_index) {
    return feedvar_element_num(feedvar_index) *
           feedvar_element_bytesize(feedvar_index);
  }

  ShapeVector feedvar_shape_nobatch(int feedvar_index) {
    if (!check_feedvar_valid(feedvar_index)) {
      return ShapeVector();
    }
    return ShapeVector{(*inVectorT_ptr)[feedvar_index].shape.begin() + 1,
                       (*inVectorT_ptr)[feedvar_index].shape.end()};
  }

  VectorOfShapeVector feedvar_shape_nobatch() {
    VectorOfShapeVector vector_of_feedvar_shape_nobatch(inVectorT_ptr->size());
    for (int index = 0; index < inVectorT_ptr->size(); ++index) {
      vector_of_feedvar_shape_nobatch.push_back(feedvar_shape_nobatch(index));
    }
    return vector_of_feedvar_shape_nobatch;
  }

  // At present, it is considered that the batch of all feedvar is consistent.
  // so for each feedvar, PaddleTensor.shape[0] should be the same.
  bool check_batch_align() {
    int batch_size_align = feedvar_batch_size(0);
    for (int feedvar_index = 0; feedvar_index < inVectorT_ptr->size();
         ++feedvar_index) {
      if (feedvar_batch_size(feedvar_index) != batch_size_align) {
        return 0;
      }
    }
    /*
    for(int fetchvar_index = 0; fetchvar_index < outVectorT_ptr->size();
    ++fetchvar_index) {
      if(fetchvar_batch_size(fetchvar_index) != batch_size_align) {
        return 0;
      }
    }
    */
    return 1;
  }

  size_t batch_size() {
    if (check_batch_align()) {
      return feedvar_batch_size(0);
    }
    return 0;
  }
};

// `Several Task` or `part of batch in Task` can be a TaskMeta.
// Task is the original Request from User.
// For example, the batch of Task is 30. There are 4 Requests.
// The batch of BatchTasks is 100, which means we can deal 100 batch 1 time.
// TaskMeta-1:{task-1,0,30} TaskMeta-2:{task-2,0,30} TaskMeta-3:{task-3,0,30}
// but the last Task will be divided to 2 TaskMeta.
// TaskMeta-4:{task-4,0,10} TaskMeta-5:{task-4,10,30}.
// TaskMeta-1 ~ TaskMeta-4 will be inside BatchTasks-1.
// TaskMeta-5 will be inside BatchTasks-2.

// TaskMeta is necessary.
// cause we need know the the corresponding relationship between
// `batch_out`(which is in BatchTasks) and `outVectorT_ptr`(which is in Task).
// especially when 1 Task be divided into several TaskMeta and be put into
// several different BatchTasks.
template <typename TaskT>
struct TaskMeta {
  TaskMeta(TaskT* ptr, size_t start, size_t add)
      : task(ptr), begin(start), end(start + add) {}

  TaskT* task;
  size_t begin;
  size_t end;
};

// each TaskT is already include batch in itself
// BatchTasks need to combine several `small TaskMeta` into a new `big TaskT`.
// The only difference between the `big TaskT` and `small TaskT` is that
// the TaskT.inVectorT_ptr->[feedvar_index].shape[0]
// which is actually batch_size is different.
template <typename TaskT>
class BatchTasks {
 public:
  typedef typename TaskT::InType InType;
  typedef typename TaskT::OutType OutType;
  typedef TaskMeta<TaskT> TaskMetaT;

  explicit BatchTasks(size_t batch_size, bool batch_align = true)
      : _batch_size(batch_size),
        _rem_size(batch_size),
        _batch_align(batch_align) {
    _batch_in.clear();
    _batch_in_offset.clear();
    _batch_out.clear();
    _batch_out_offset.clear();
    _taskmeta_vector.clear();
  }

  ~BatchTasks() {
    _batch_in.clear();
    _batch_in_offset.clear();
    _batch_out.clear();
    _batch_out_offset.clear();
    _taskmeta_vector.clear();
  }

  // synchronized operation
  // because Upper level callers of this function have already locked.
  size_t append_task(TaskT* task) {
    size_t add = std::min(task->rem, _rem_size);
    if (!_batch_align) {
      add = task->rem;
    }
    int start_index = task->batch_size() - task->rem;
    TaskMetaT tm(task, start_index, add);
    _taskmeta_vector.push_back(tm);

    task->rem -= add;
    _rem_size -= add;
    return _rem_size;
  }

  static bool check_valid(const typename TaskT::InVectorT& in,
                          const typename TaskT::OutVectorT& out,
                          bool align) {
    (void)in;
    (void)out;
    (void)align;
    return true;
  }

  // this should be modified totally.
  // maybe we don`t need to do this inside the BatchTasks.
  // we can do the copy work outside the BatchTasks.
  // cause maybe next time we don`t need to do the extra copy.
  // directly copy the every Task into the Predictor.

  // lod is not supported.
  // if lod is set, we should not allow to use the bsf task.

  // batch.merge_tasks() is thread-safe function
  // cause batch is a local variable and Task is just read, not written.
  void merge_tasks() {
    if (_taskmeta_vector.size() <= 0) {
      return;
    }

    // Temporarily, the batch of each feedvar is consistent
    // If not consistent, use feedvar_batch_size instead of task->batch_size().
    int temp_batch = 0;
    for (size_t ti = 0; ti < _taskmeta_vector.size(); ++ti) {
      TaskMetaT& tm = _taskmeta_vector[ti];
      temp_batch += tm.task->batch_size();
    }
    if (temp_batch > _batch_size) {
      LOG(ERROR) << "_realNumber_batch_in >_batch_size, error.";
      return;
    }

    int feedvar_num = _taskmeta_vector[0].task->inVectorT_ptr->size();
    if (_batch_in_offset.size() == 0) {
      _batch_in_offset.resize(feedvar_num, 0);
      _realNumber_batch_in.resize(feedvar_num, temp_batch);
    }

    for (size_t ti = 0; ti < _taskmeta_vector.size(); ++ti) {
      TaskMetaT& tm = _taskmeta_vector[ti];

      for (int index = 0; index < feedvar_num; ++index) {
        const paddle::PaddleTensor& feedVarTensor =
            (*tm.task->inVectorT_ptr)[index];
        size_t feedvar_bytesize = tm.task->feedvar_bytesize(index);

        if (ti == 0) {
          if (feedVarTensor.lod.size() > 0 && feedVarTensor.lod[0].size() > 0) {
            LOG(ERROR) << "lod Tensor is not supported now.";
            return;
          }
          // for now, we assume that every task feedvar_bytesize is the same.
          // which means we dont support auto embedding.
          // but for different feedvar, it is different.
          paddle::PaddleTensor paddleTensor;
          paddleTensor.dtype = feedVarTensor.dtype;
          paddleTensor.name = feedVarTensor.name;
          paddleTensor.lod = feedVarTensor.lod;
          paddleTensor.shape = feedVarTensor.shape;
          paddleTensor.shape[0] = _realNumber_batch_in[index];
          paddleTensor.data.Resize(feedvar_bytesize *
                                   _realNumber_batch_in[index]);
          _batch_in.push_back(paddleTensor);
        }

        void* dst_ptr = _batch_in[index].data.data() + _batch_in_offset[index];
        void* source_ptr =
            feedVarTensor.data.data() + feedvar_bytesize * tm.begin;
        size_t length = feedvar_bytesize * (tm.end - tm.begin);
        memcpy(dst_ptr, source_ptr, length);
        _batch_in_offset[index] += length;
      }
    }
  }

  bool check_fetchvar_valid(int fetchvar_index) {
    if (fetchvar_index < 0 || _batch_out.size() <= fetchvar_index) {
      LOG(ERROR) << "fetchvar doesnt exsit or fetchvar_index error";
      return 0;
    }

    if (_batch_out[fetchvar_index].shape.size() <= 0) {
      LOG(ERROR) << "fetchvar[" << fetchvar_index << "].shape.size()<=0,error";
      return 0;
    }

    return 1;
  }

  size_t fetchvar_batch_size(int fetchvar_index) {
    if (!check_fetchvar_valid(fetchvar_index)) {
      return 0;
    }

    return _batch_out[fetchvar_index].shape[0];
  }

  size_t fetchvar_element_bytesize(int fetchvar_index) {
    if (!check_fetchvar_valid(fetchvar_index)) {
      return 0;
    }
    int dtype = _batch_out[fetchvar_index].dtype;
    if (dtype == paddle::PaddleDType::INT64) {
      return sizeof(int64_t);
    }
    if (dtype == paddle::PaddleDType::FLOAT32) {
      return sizeof(float);
    }
    if (dtype == paddle::PaddleDType::INT32) {
      return sizeof(int32_t);
    }
    if (dtype == paddle::PaddleDType::UINT8) {
      return sizeof(char);
    }
    return 0;
  }

  // Now, the implementation of this function is based on assumption
  // that shape [0] = batch_size.
  size_t fetchvar_element_num(int fetchvar_index) {
    if (!check_fetchvar_valid(fetchvar_index)) {
      return 0;
    }
    size_t element_num = 1;
    if (_batch_out[fetchvar_index].shape.size() == 1) {
      // cause shape[0] is batch_size.
      return 1;
    }
    // start from shape[1], cause shape[0] = batch_size.
    for (int i = 1; i < _batch_out[fetchvar_index].shape.size(); ++i) {
      element_num *= _batch_out[fetchvar_index].shape[i];
    }
    return element_num;
  }

  size_t fetchvar_bytesize(int fetchvar_index) {
    return fetchvar_element_num(fetchvar_index) *
           fetchvar_element_bytesize(fetchvar_index);
  }

  bool check_fetchvar_batch_align() {
    int batch_size_align = fetchvar_batch_size(0);

    for (int fetchvar_index = 0; fetchvar_index < _batch_out.size();
         ++fetchvar_index) {
      if (fetchvar_batch_size(fetchvar_index) != batch_size_align) {
        return 0;
      }
    }

    return 1;
  }

  size_t fetchvar_batch_size() {
    if (check_fetchvar_batch_align()) {
      return fetchvar_batch_size(0);
    }
    return 0;
  }

  void notify_tasks() {
    if (_taskmeta_vector.size() <= 0) {
      LOG(ERROR) << "_taskmeta_vector.size() <=0, error.";
      return;
    }
    if (_realNumber_batch_in[0] != fetchvar_batch_size()) {
      LOG(ERROR) << "_batch_out`s batch != _batch_in`s batch, error.";
      return;
    }

    int fetchvar_num = _batch_out.size();
    if (_batch_out_offset.size() == 0) {
      _batch_out_offset.resize(fetchvar_num, 0);
    }

    for (size_t ti = 0; ti < _taskmeta_vector.size(); ++ti) {
      TaskT* task = _taskmeta_vector[ti].task;
      size_t begin = _taskmeta_vector[ti].begin;
      size_t end = _taskmeta_vector[ti].end;
      size_t add = end - begin;

      for (int index = 0; index < fetchvar_num; ++index) {
        // the task->outVectorT_ptr is null before core->run().
        // first time we should copy from _batch_out
        // so we need init.
        size_t fetchvar_bytesize_index = fetchvar_bytesize(index);
        if (task->outVectorT_ptr->size() <= index) {
          paddle::PaddleTensor tensor_out;
          tensor_out.name = _batch_out[index].name;
          tensor_out.dtype = paddle::PaddleDType(_batch_out[index].dtype);
          tensor_out.shape = _batch_out[index].shape;
          tensor_out.shape[0] = task->batch_size();
          tensor_out.lod = _batch_out[index].lod;
          // resize all batch memory at one time
          size_t databuf_size = task->batch_size() * fetchvar_bytesize_index;
          tensor_out.data.Resize(databuf_size);
          task->outVectorT_ptr->push_back(tensor_out);
        }

        paddle::PaddleTensor& fetchVarTensor = (*task->outVectorT_ptr)[index];

        void* dst_ptr =
            fetchVarTensor.data.data() + fetchvar_bytesize_index * begin;
        size_t length = fetchvar_bytesize_index * add;
        if (_batch_out_offset[index] + length >
            fetchvar_batch_size() * fetchvar_bytesize(index)) {
          LOG(ERROR) << "_batch_out is less than taskmeta, error.";
          return;
        }
        void* source_ptr =
            _batch_out[index].data.data() + _batch_out_offset[index];

        memcpy(dst_ptr, source_ptr, length);
        _batch_out_offset[index] += length;
      }

      size_t index = task->index.fetch_add(add);
      if ((index + add) >= task->batch_size()) {
        char c = 0;
        while (write(task->write_fd, &c, 1) != 1 && errno == EINTR) {
        }
        butil::return_object(task);
      }
    }
  }

  const typename TaskT::InVectorT& in() const { return _batch_in; }

  typename TaskT::OutVectorT& out() { return _batch_out; }

  size_t task_size() { return _taskmeta_vector.size(); }

 private:
  std::vector<TaskMetaT> _taskmeta_vector;
  typename TaskT::InVectorT _batch_in;
  std::vector<size_t> _batch_in_offset;
  std::vector<size_t> _realNumber_batch_in;
  typename TaskT::OutVectorT _batch_out;
  std::vector<size_t> _batch_out_offset;
  std::vector<size_t> _realNumber_batch_out;
  size_t _rem_size;
  size_t _batch_size;
  bool _batch_align;
};

// BSF task handle
// TaskHandler is the handle of Task.
// `read_fd` is used for receive signal in brpc Thread.
// 'write_fd' is used for write signal in bsf Thread.
// when TaskMeta is done, bsf Thread will write to 'write_fd'.
// brpc Thread is keeping reading 'read_fd' in a while loop.
// brpc Thread will receive signal when TaskMeta is done.
// so `read_fd` and 'write_fd' is used for communicate in different Thread.
template <typename TaskT>
struct TaskHandler {
  int read_fd;
  int write_fd;

  TaskHandler() : read_fd(-1), write_fd(-1) {
    // do nothing
  }

  explicit TaskHandler(TaskT const& task)
      : read_fd(task.read_fd), write_fd(task.write_fd) {
    // do nothing
  }

  inline bool valid() const { return read_fd >= 0 && write_fd >= 0; }

  static TaskHandler<TaskT>& valid_handle() {
    static TaskHandler<TaskT> vhandle;
    return vhandle;
  }
};

// TaskExecutor is a Thread pool.
template <typename TaskT>
class TaskExecutor;

// ThreadContext is used for start a bsf Thread.
template <typename TaskT>
struct ThreadContext {
  TaskExecutor<TaskT>* executor;
  void* user_thread_context;
  THREAD_T tid;
  int init_status;

  ThreadContext()
      : executor(NULL), user_thread_context(NULL), tid(-1), init_status(0) {
    // do nothing
  }

  ~ThreadContext() {
    tid = -1;
    executor = NULL;
    user_thread_context = NULL;
    init_status = 0;
  }
};

// TaskExecutor is a Thread pool.
// Each Model corresponding to a Model.
// TaskT is actually a Request preprocessed by ReaderOp.
// TaskT will be divided as TaskMeta which will be
// put into _task_queue in brpc-Thread by schedule().
// TaskHander will be returned to brpc-Thread.
// start() function will create `thread_num` bsf Threads.
// every bsf Thread check the _task_queue and take TaskMeta from it.
// when a Task`s all TaskMeta is done, TaskHander will be noticed.
template <typename TaskT>
class TaskExecutor {
 public:
  typedef typename TaskT::InType InType;
  typedef typename TaskT::OutType OutType;
  typedef typename TaskT::InVectorT InVectorT;
  typedef typename TaskT::OutVectorT OutVectorT;
  typedef std::vector<TaskT> TaskArrayT;
  typedef baidu::paddle_serving::predictor::MempoolWrapper MempoolWrapper;

  TaskExecutor()
      : _stop(false),
        _thread_init_fn(NULL),
        _thread_reset_fn(NULL),
        _user_thread_contexts(NULL),
        _batch_size(DEFAULT_BATCH_SIZE),
        _batch_align(false),
        _fn(NULL) {
    THREAD_MUTEX_INIT(&_mut, NULL);
    THREAD_COND_INIT(&_cond, NULL);
    _task_queue.clear();
  }

  ~TaskExecutor() {
    THREAD_MUTEX_DESTROY(&_mut);
    THREAD_COND_DESTROY(&_cond);
  }

  // cause vector.resize will use copy or move construct.
  TaskExecutor(TaskExecutor<TaskT>&& other) noexcept {
    if (this != &other) {
      TaskExecutor();
    }
  }

  void set_batch_size(size_t batch_size) { _batch_size = batch_size; }

  void set_batch_align(size_t batch_align) { _batch_align = batch_align; }

  void set_thread_init_fn(boost::function<int(void*)> init_fn,
                          void** contexts = NULL) {
    _thread_init_fn = init_fn;
    _user_thread_contexts = contexts;
  }

  void set_thread_reset_fn(boost::function<int(void*)> reset_fn) {
    _thread_reset_fn = reset_fn;
  }

  void set_thread_callback_fn(boost::function<void(const void*, void*)> cb) {
    _fn = cb;
  }

  int start(uint32_t thread_num, uint32_t init_timeout_sec = 0);
  void stop();

  static void* thread_entry(void* args);

  int work(ThreadContext<TaskT>* context);

  TaskHandler<TaskT> schedule(const void*, void*);

  bool move_task_to_batch(BatchTasks<TaskT>& batch);  // NOLINT

 private:
  TaskExecutor(TaskExecutor<TaskT> const& other) = delete;

  TaskExecutor& operator=(TaskExecutor<TaskT> const& other) = delete;
  /*
  TaskExecutor(TaskExecutor<TaskT> && other) = delete;

  TaskExecutor& operator=(TaskExecutor<TaskT> && other) = delete;
  */

  bool _stop;

  // can't use boost::mutex, because some stupid macro
  THREAD_MUTEX_T _mut;
  THREAD_COND_T _cond;

  std::list<TaskT*> _task_queue;

  boost::function<int(void*)> _thread_init_fn;
  boost::function<int(void*)> _thread_reset_fn;
  void** _user_thread_contexts;

  std::vector<ThreadContext<TaskT>*> _thread_contexts;

  size_t _batch_size;
  bool _batch_align;

  boost::function<void(const void*, void*)> _fn;
};

// TaskExecutorVector is a SingleTon class.
// Each Model corresponding to a TaskExecutor.
// So we need several TaskExecutor when there are more than 1 Model.
template <typename TaskT>
class TaskExecutorVector {
 public:
  static TaskExecutorVector<TaskT>& instance() {
    static TaskExecutorVector<TaskT> singleton;
    return singleton;
  }

  void resize(int size) { _vector_executor.resize(size); }

  TaskExecutor<TaskT>& operator[](int index) {
    if (_vector_executor.size() <= index || index <= -1) {
      LOG(ERROR) << "_vector_executor.size() <= index or <= -1";
      throw "_vector_executor.size() <= index or <= -1";
    }
    return _vector_executor[index];
  }

 private:
  TaskExecutorVector() = default;
  TaskExecutorVector(const TaskExecutorVector<TaskT>& other) = delete;
  TaskExecutorVector& operator=(const TaskExecutorVector<TaskT>& other) =
      delete;
  TaskExecutorVector(TaskExecutorVector<TaskT>&& other) = delete;
  TaskExecutorVector& operator=(TaskExecutorVector<TaskT>&& other) = delete;
  std::vector<TaskExecutor<TaskT>> _vector_executor;
};

// TaskManager is actually a wrapper of Request in bsf.
// TaskManager`s schedule() change Request to be TaskT.
// and divided TaskT into several TaskMeta to put into the TaskExecutor`s
// task_queue.
// wait() is a while loop to receive signal when a whole Task is done.
template <typename InItemT, typename OutItemT>
class TaskManager {
 public:
  typedef Task<InItemT, OutItemT> TaskT;
  typedef typename TaskT::InVectorT InVectorT;
  typedef typename TaskT::OutVectorT OutVectorT;

  explicit TaskManager(uint32_t index)  // NOLINT
      : _model_index(index) {}

  ~TaskManager() { wait(); }

  bool schedule(const void* in, void* out);  // NOLINT
  void wait();

  inline void clear() { wait(); }

 private:
  TaskHandler<TaskT> _task_owned;
  uint32_t _model_index;
};  // class TaskManager

class AutoMutex {
 public:
  explicit AutoMutex(THREAD_MUTEX_T& mut) : _mut(mut) {
    THREAD_MUTEX_LOCK(&_mut);
  }

  ~AutoMutex() { THREAD_MUTEX_UNLOCK(&_mut); }

 private:
  THREAD_MUTEX_T& _mut;
};

}  // namespace bsf
}  // namespace im

// #include "core/predictor/framework/bsf-inl-tensor.h"
#include "core/predictor/framework/bsf-inl.h"
