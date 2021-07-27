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
#include <deque>
#include <vector>

#ifdef BCLOUD
#include "base/atomicops.h"
#else
#include "butil/atomicops.h"
#endif

#include "core/predictor/common/inner_common.h"

#include "boost/function.hpp"

namespace im {
namespace bsf {

static const size_t DEFAULT_BATCH_SIZE = 100;

template <typename InItemT, typename OutItemT>
struct Task {
  typedef std::vector<InItemT> InArrayT;
  typedef std::vector<OutItemT> OutArrayT;
  typedef InItemT InType;
  typedef OutItemT OutType;
  typedef Task<InItemT, OutItemT> TaskT;

  int read_fd;
  int write_fd;

  pid_t owner_tid;

  const InArrayT* in;
  OutArrayT* out;

  size_t rem;
  size_t size;

  size_t batch_size() { return in->size(); }

  butil::atomic<size_t> index;

  Task() {
    read_fd = -1;
    write_fd = -1;
    owner_tid = -1;
    in = NULL;
    out = NULL;
    rem = -1;
    size = -1;
    index.store(0, butil::memory_order_relaxed);
  }
};

template <typename TaskT>
struct TaskMeta {
  TaskMeta(TaskT* ptr, size_t start, size_t add)
      : task(ptr), begin(start), end(start + add) {}

  TaskT* task;
  size_t begin;
  size_t end;
};

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
    _batch_out.clear();
    _tasks.clear();
  }

  ~BatchTasks() {
    _batch_in.clear();
    _batch_out.clear();
    _tasks.clear();
  }

  // synchronized operation
  size_t append_task(TaskT* task) {
    size_t add = std::min(task->rem, _rem_size);
    if (!_batch_align) {
      add = task->rem;
    }

    TaskMetaT tm(task, task->in->size() - task->rem, add);
    _tasks.push_back(tm);

    task->rem -= add;
    _rem_size -= add;
    return _rem_size;
  }

  static bool check_valid(const typename TaskT::InArrayT& in,
                          const typename TaskT::OutArrayT& out,
                          bool align) {
    (void)in;
    (void)out;
    (void)align;
    return true;
  }

  void merge_tasks() {
    for (size_t ti = 0; ti < _tasks.size(); ++ti) {
      TaskMetaT& tm = _tasks[ti];
      for (size_t vi = tm.begin; vi < tm.end; ++vi) {
        _batch_in.push_back((*tm.task->in)[vi]);
        _batch_out.push_back((*tm.task->out)[vi]);
      }
    }
  }

  void notify_tasks() {
    if (_batch_out.size() != _batch_in.size()) {
      LOG(ERROR) << "batch size not consistency: " << _batch_out.size()
                 << " != " << _batch_in.size();
      return;
    }

    for (size_t ti = 0, bi = 0; ti < _tasks.size(); ++ti) {
      TaskT* task = _tasks[ti].task;
      size_t begin = _tasks[ti].begin;
      size_t end = _tasks[ti].end;
      size_t add = end - begin;

      for (size_t oi = begin; oi < end; ++oi, ++bi) {
        if (bi >= _batch_in.size()) {
          LOG(ERROR) << "batch index overflow: " << bi << " > "
                     << _batch_in.size();
          return;
        }
        (*task->out)[oi] = _batch_out[bi];
      }

      size_t index = task->index.fetch_add(add);
      if ((index + add) >= task->in->size()) {
        char c = 0;
        while (write(task->write_fd, &c, 1) != 1 && errno == EINTR) {
        }
        butil::return_object(task);
      }
    }
  }

  const typename TaskT::InArrayT& in() const { return _batch_in; }

  typename TaskT::OutArrayT& out() { return _batch_out; }

  size_t task_size() { return _tasks.size(); }

 private:
  std::vector<TaskMetaT> _tasks;
  typename TaskT::InArrayT _batch_in;
  typename TaskT::OutArrayT _batch_out;
  size_t _rem_size;
  size_t _batch_size;
  bool _batch_align;
};

// BSF task handle
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

template <typename TaskT>
class TaskExecutor;

template <typename InItemT, typename OutItemT>
class TaskManager;

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

template <typename TaskT>
class TaskExecutor {
 public:
  typedef typename TaskT::InType InType;
  typedef typename TaskT::OutType OutType;
  typedef typename TaskT::InArrayT InArrayT;
  typedef typename TaskT::OutArrayT OutArrayT;
  typedef std::vector<TaskT> TaskArrayT;

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

  static TaskExecutor<TaskT>* instance() {
    static TaskExecutor<TaskT> singleton;
    return &singleton;
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

  void set_thread_callback_fn(
      boost::function<void(const InArrayT&, OutArrayT&)> cb) {
    _fn = cb;
  }

  int start(uint32_t thread_num, uint32_t init_timeout_sec = 0);
  void stop();

  static void* thread_entry(void* args);

 private:
  TaskExecutor(TaskExecutor<TaskT> const& other);
  TaskExecutor* operator=(TaskExecutor<TaskT> const& other);

  int work(ThreadContext<TaskT>* context);

  TaskHandler<TaskT> schedule(const InArrayT&, OutArrayT&);

  bool fetch_batch(BatchTasks<TaskT>& batch);  // NOLINT

  bool _stop;

  // can't use boost::mutex, because some stupid macro
  THREAD_MUTEX_T _mut;
  THREAD_COND_T _cond;

  std::deque<TaskT*> _task_queue;

  boost::function<int(void*)> _thread_init_fn;
  boost::function<int(void*)> _thread_reset_fn;
  void** _user_thread_contexts;

  std::vector<ThreadContext<TaskT>*> _thread_contexts;
  friend class TaskManager<InType, OutType>;

  size_t _batch_size;
  bool _batch_align;

  boost::function<void(const InArrayT&, OutArrayT&)> _fn;
};

template <typename InItemT, typename OutItemT>
class TaskManager {
 public:
  typedef Task<InItemT, OutItemT> TaskT;
  typedef typename TaskT::InArrayT InArrayT;
  typedef typename TaskT::OutArrayT OutArrayT;

  explicit TaskManager(TaskExecutor<TaskT>& exe, size_t batch_size)  // NOLINT
      : _executor(exe) {}

  TaskManager() : _executor(*TaskExecutor<TaskT>::instance()) {}

  ~TaskManager() { wait(); }

  bool schedule(const InArrayT& in, OutArrayT& out);  // NOLINT
  void wait();

  inline void clear() { wait(); }

 private:
  TaskExecutor<TaskT>& _executor;
  TaskHandler<TaskT> _task_owned;
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

#include "core/predictor/framework/bsf-inl-tensor.h"
#include "core/predictor/framework/bsf-inl.h"
