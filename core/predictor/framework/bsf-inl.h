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

#ifdef BCLOUD
#include <base/atomicops.h>
#else
#include <butil/atomicops.h>
#endif

#include <sys/syscall.h>
#include <boost/bind.hpp>

#include "core/predictor/common/inner_common.h"
#include "core/predictor/framework/memory.h"

namespace im {
namespace bsf {

template <typename TaskT>
void* TaskExecutor<TaskT>::thread_entry(void* args) {
  ThreadContext<TaskT>* context = static_cast<ThreadContext<TaskT>*>(args);
  TaskExecutor<TaskT>* executor =
      static_cast<TaskExecutor<TaskT>*>(context->executor);
  executor->work(context);

  return nullptr;
}

template <typename TaskT>
int TaskExecutor<TaskT>::start(uint32_t thread_num, uint32_t init_timeout_sec) {
  _stop = false;
  if (!_thread_contexts.empty()) {
    LOG(WARNING) << "BSF has started";
    return 0;
  }

  if (thread_num == 0) {
    LOG(ERROR) << "cannot init BSF with zero thread";
    return -1;
  }

  ThreadContext<TaskT>* contexts = new ThreadContext<TaskT>[thread_num];
  for (uint32_t i = 0; i < thread_num; ++i) {
    contexts[i].executor = this;
    if (_user_thread_contexts != NULL) {
      contexts[i].user_thread_context = _user_thread_contexts[i];
    }

    int rc = THREAD_CREATE(
        &contexts[i].tid, NULL, &TaskExecutor::thread_entry, &contexts[i]);
    if (rc != 0) {
      LOG(ERROR) << "failed to create BSF worker thread: index=" << i
                 << ", rc=" << rc << ", errno=" << errno << ":"
                 << strerror(errno);
      return -1;
    }

    _thread_contexts.push_back(&contexts[i]);
  }

  int init_timeout = init_timeout_sec * 1000 * 1000;
  bool has_error = false;

  bool has_timeout = true;
  if (init_timeout == 0) {
    has_timeout = false;
  }

  while (!has_timeout || init_timeout > 0) {
    bool done = true;
    for (size_t i = 0; i < _thread_contexts.size(); ++i) {
      if (_thread_contexts[i]->init_status < 0) {
        has_error = true;
        break;
      }

      if (_thread_contexts[i]->init_status == 0) {
        done = false;
      }
    }

    if (has_error) {
      LOG(ERROR) << "BSF thread init error";
      return -1;
    }

    if (done) {
      LOG(INFO) << "BSF thread init done";
      return 0;
    }

    // 100ms
    const int sleep_interval = 100 * 1000;
    usleep(sleep_interval);
    init_timeout -= sleep_interval;
  }

  LOG(ERROR) << "BSF thread init timed out";
  return -1;
}

template <typename TaskT>
void TaskExecutor<TaskT>::stop() {
  _stop = true;
  for (size_t i = 0; i < _thread_contexts.size(); ++i) {
    THREAD_CANCEL(_thread_contexts[i]->tid);
  }

  for (size_t i = 0; i < _thread_contexts.size(); ++i) {
    THREAD_JOIN(_thread_contexts[i]->tid, NULL);
  }
  _thread_contexts.clear();
}

template <typename TaskT>
TaskHandler<TaskT> TaskExecutor<TaskT>::schedule(
    const void* inVectorT_ptr,
    void* outVectorT_ptr) {  // NOLINT
  TaskT* task = butil::get_object<TaskT>();
  if (!task) {
    LOG(ERROR) << "Failed get TaskT from object pool";
    return TaskHandler<TaskT>::valid_handle();
  }

  /*
  if (!BatchTasks<TaskT>::check_valid(in, out, _batch_align)) {
    LOG(ERROR) << "Invalid input & output";
    return TaskHandler<TaskT>::valid_handle();
  }
  */

  int fds[2];
  int rc = pipe(fds);
  if (rc != 0) {
    LOG(ERROR) << "call pipe() failed, errno=" << errno << ":"
               << strerror(errno);
    return TaskHandler<TaskT>::valid_handle();
  }

  task->read_fd = fds[0];
  task->write_fd = fds[1];
  task->owner_tid = ::syscall(SYS_gettid);

  task->inVectorT_ptr = (const InVectorT*)inVectorT_ptr;
  task->outVectorT_ptr = (OutVectorT*)outVectorT_ptr;
  task->rem = task->batch_size();
  task->index.store(0, butil::memory_order_relaxed);

  AutoMutex lock(_mut);
  _task_queue.push_back(task);
  THREAD_COND_SIGNAL(&_cond);

  return TaskHandler<TaskT>(*task);
}

// this function is accessed by multi thread.
// so AutoMutex at first.
// so batch.append_task is thread safe.
// you dont need to add extra lock in append_task()
template <typename TaskT>
bool TaskExecutor<TaskT>::move_task_to_batch(
    BatchTasks<TaskT>& batch) {  // NOLINT
  AutoMutex lock(_mut);
  while (_task_queue.empty()) {
    THREAD_COND_WAIT(&_cond, &_mut);
  }

  if (_task_queue.empty()) {
    LOG(ERROR) << "invalid task queue!";
    return false;
  }

  while (!_task_queue.empty()) {
    TaskT* task = _task_queue.front();
    size_t rem = batch.append_task(task);
    if (task->rem <= 0) {
      _task_queue.pop_front();
    }
    if (rem <= 0) break;
  }

  return true;
}

// this function is accessed by multi thread.
// move_task_to_batch have add lock inside the function.
// Packaging 1 TaskT as 1 or Several TaskMeta.
// TaskT is from the SingleTon TaskExecutor`s _task_queue
// although TaskMeta is a local variable, but several TaskMeta may points to
// the same TaskT which is get from the SingleTon TaskExecutor`s _task_queue.
// put TaskMeta to the local variable BatchTasks<TaskT> batch.

// batch.merge_tasks() and batch.notify_tasks() has no lock.
// BatchTasks<TaskT> batch itself is a local variable, it`s thread safe.
// If batch.merge_tasks() and batch.notify_tasks() do something to TaskMeta
// you need to pay attention to that.
// Multi-Thread deal with different TaskMeta(cause it`s created as local
// variable)
// But different TaskMeta may points to the same TaskT
// which is get from the SingleTon TaskExecutor`s _task_queue.

template <typename TaskT>
int TaskExecutor<TaskT>::work(ThreadContext<TaskT>* context) {
  if (MempoolWrapper::instance().thread_initialize() != 0) {
    LOG(ERROR) << "Failed thread initialize mempool";
    return -1;
  }

  if (_thread_init_fn != NULL) {
    if (_thread_init_fn(context->user_thread_context) != 0) {
      LOG(ERROR) << "execute thread init thunk failed, BSF thread will exit";
      context->init_status = -1;
      return -1;
    } else {
      LOG(INFO) << "execute thread init thunk succeed";
    }
  }

  context->init_status = 1;
  while (!_stop) {
    if (_thread_reset_fn != NULL) {
      if (_thread_reset_fn(context->user_thread_context) != 0) {
        LOG(ERROR) << "execute user thread reset failed";
      }
    }

    if (MempoolWrapper::instance().thread_clear() != 0) {
      LOG(ERROR) << "Failed thread clear mempool";
      return -1;
    }

    BatchTasks<TaskT> batch(_batch_size, _batch_align);
    if (move_task_to_batch(batch)) {
      batch.merge_tasks();
      _fn(&batch.in(), &batch.out());
      batch.notify_tasks();
    }
  }

  return 0;
}

template <typename InItemT, typename OutItemT>
bool TaskManager<InItemT, OutItemT>::schedule(const void* in,
                                              void* out) {  // NOLINT
  TaskHandler<TaskT> handler =
      TaskExecutorVector<TaskT>::instance()[_model_index].schedule(in, out);

  if (handler.valid()) {
    _task_owned = handler;
    return true;
  } else {
    LOG(ERROR) << "failed to schedule task";
    return false;
  }
}

template <typename InItemT, typename OutItemT>
void TaskManager<InItemT, OutItemT>::wait() {
  char buffer[128];
  while (read(_task_owned.read_fd, buffer, sizeof(buffer)) < 0 &&
         errno == EINTR) {
  }

  close(_task_owned.read_fd);
  close(_task_owned.write_fd);

  _task_owned.read_fd = -1;
  _task_owned.write_fd = -1;
  return;
}
}  // namespace bsf
}  // namespace im
