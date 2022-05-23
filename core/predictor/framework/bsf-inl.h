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

// this file is included by bsf.h
namespace im {
namespace bsf {

template <typename InItemT, typename OutItemT>
bool Task<InItemT, OutItemT>::task_fetch_init(BatchTasks<TaskT>& batchTask) {
  // 双检锁，减少加锁的粒度
  if (!fetch_init) {
    if (total_taskmeta_num > 1) {
      // 对于task被拆分为多个taskmeta,需要加锁。
      AutoMutex lock(task_mut);
      task_fetch_create(batchTask);
    } else {
      // 对于task只有1个taskmeta,不需要加锁。
      task_fetch_create(batchTask);
    }
  }
  return true;
}

template <typename InItemT, typename OutItemT>
bool Task<InItemT, OutItemT>::task_fetch_create(BatchTasks<TaskT>& batchTask) {
  if (!fetch_init) {
    vector_fetch_lod_index = batchTask.vector_fetch_lod_index;
    set_fetch_nobatch_index = batchTask.set_fetch_nobatch_index;
    OutVectorT taskMetaOutLodTensor;
    size_t fetchvar_num = batchTask._batch_out.size();
    for (size_t fetchvar_index = 0; fetchvar_index < fetchvar_num;
         ++fetchvar_index) {
      size_t fetchvar_bytesize_index =
          batchTask.fetchvar_bytesize(fetchvar_index);
      size_t fetchvar_batch = 0;
      // 1. nobatch fetchvar情况
      if (set_fetch_nobatch_index.size() > 0 &&
          set_fetch_nobatch_index.find(fetchvar_index) !=
              set_fetch_nobatch_index.end()) {
        fetchvar_batch = 1;
      } else if (vector_fetch_lod_index.size() > 0 &&
                 std::find(vector_fetch_lod_index.begin(),
                           vector_fetch_lod_index.end(),
                           fetchvar_index) != vector_fetch_lod_index.end()) {
        // lod fetchvar情况，此时无法确定总的shape[0]
        // 根据task中的task_num总数开辟task_num个临时空间
        // 每个lod型的fetchvar拷贝到对应的临时空间中
        // 最后再计算临时空间的总量，合并fetchvar和lod
        fetchvar_batch = 0;
      } else {
        // 普通fetchvar情况，此时该Task总的fetchvar_batch =
        // 输入的总的batch_size()
        fetchvar_batch = batch_size();
      }
      paddle::PaddleTensor tensor_out;
      tensor_out.name = batchTask._batch_out[fetchvar_index].name;
      tensor_out.dtype =
          paddle::PaddleDType(batchTask._batch_out[fetchvar_index].dtype);
      tensor_out.shape = batchTask._batch_out[fetchvar_index].shape;
      tensor_out.shape[0] = fetchvar_batch;
      if (fetchvar_batch != 0) {
        // 此时 lod 为空。
        tensor_out.lod = batchTask._batch_out[fetchvar_index].lod;
        // resize all batch memory at one time

        size_t databuf_size = fetchvar_batch * fetchvar_bytesize_index;

        void* databuf_data =
            MempoolWrapper::instance().malloc(databuf_size, memoryPtr);
        paddle::PaddleBuf paddleBuf(databuf_data, databuf_size);
        tensor_out.data = paddleBuf;

        // tensor_out.data.Resize(databuf_size);
      } else {
        // 当taskmeta_num = 1时，由于同时只有一个taskMeta操作task
        // 不涉及线程安全问题，所以此时可以直接由taskMeta->task->resize->copy

        // 当task被分为多个taskMeta时，需要临时对象记录
        // 收齐后再一起合并
        if (total_taskmeta_num > 1) {
          taskMetaOutLodTensor.push_back(tensor_out);
        }
      }
      outVectorT_ptr->push_back(tensor_out);
    }
    // outLodTensorVector实际是一个双层vector
    // shape为taskmeta_num * vector_fetch_lod_index.size();
    outLodTensorVector.resize(total_taskmeta_num, taskMetaOutLodTensor);
    fetch_init = true;
  }
  return true;
}

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

  size_t init_timeout = init_timeout_sec * 1000 * 1000;
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
    const size_t sleep_interval = 100 * 1000;
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
    void* outVectorT_ptr,
    MempoolRegion* memoryPtr) {  // NOLINT
  TaskT* task = butil::get_object<TaskT>();
  if (!task) {
    LOG(ERROR) << "Failed get TaskT from object pool";
    return TaskHandler<TaskT>::valid_handle();
  }
  task->clear();

  /*
  if (!BatchTasks<TaskT>::check_valid(in, out, _overrun)) {
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
  task->memoryPtr = memoryPtr;
  // task->_bspec_key = _bspec_key;
  task->inVectorT_ptr = (const InVectorT*)inVectorT_ptr;
  task->outVectorT_ptr = (OutVectorT*)outVectorT_ptr;
  if (!task->task_init()) {
    LOG(ERROR) << "task->init() failed";
  }
  task->rem = task->batch_size();
  task->index.store(0, butil::memory_order_relaxed);
  AutoMutex lock(_mut);
  _task_queue.push_back(task);
  THREAD_COND_SIGNAL(&_cond);

  return TaskHandler<TaskT>(*task);
}

// this function is accessed by multi thread.
// so AutoMutex at first.
// so batchTask.append_task is thread safe.
// you dont need to add extra lock in append_task()
// task is already init.
template <typename TaskT>
bool TaskExecutor<TaskT>::move_task_to_batch(
    BatchTasks<TaskT>& batchTask) {  // NOLINT
  AutoMutex lock(_mut);
  while (_task_queue.empty()) {
    THREAD_COND_WAIT(&_cond, &_mut);
  }

  if (_task_queue.empty()) {
    LOG(ERROR) << "invalid task queue!";
    return false;
  }

  TaskT* previous_task = nullptr;
  int padding_task_count = 0;
  while (!_task_queue.empty()) {
    TaskT* task = _task_queue.front();

    // 由于无法确定fetchVar是否为lod（即使输入是非lod，输出也可能是lod）
    // 简单的处理方法是：task不能被拆分，即用户的请求可以合并一起预测，但不能拆分两个小部分去预测。
    // 只需要设置engine的属性allow_split_request = false即可。

    // 复杂的处理方法是允许拆分Task，无论是否包含lod.
    // 难点：预测前，能够知道被拆成了几个taskmeta,但只有预测后，才知道有多少个fetchvar,多少个lod的fetchvar
    // 所以，task中先要创建taskmeta_num* fetchvar
    // num（lod类型的）个临时PaddleTensor（存储data及Lod）
    // 由于多线程调度的单位是taskmeta，故只能在notify_task中，用taskmeta->task去创建
    // 此时由于多个taskmeta对应一个task，存在多线程竞争，所以需要在task中加锁。
    // 原子操作不可行，因为多个线程必须等待创建好上述的PaddleTensor后才能继续。
    // 对于普通的fetch，也需要加锁去创建PaddleTensor，后续才能往里拷贝。

    // _overrun表示，异步BatchTasks是否允许单次临时超过限制。
    // _overrun为true时，即使BatchTasks剩下1-batch，也会全放入一个完整的Task，允许临时超限。
    // _overrun为false时，不允许。
    // 对于模型本身有最大Batch限制的情况，应将该值设为false，默认为false。
    // 对于模型本身无最大Batch限制，但自己设置了BatchTasks的最大Batch，可以考虑设置为True。

    // _allow_split_request ==
    // true，则允许拆分task.BatchTasks剩下1-batch，则会从下一个Task中拆出1-Batch
    // _allow_split_request ==
    // false，则每个task不会被拆分。BatchTasks剩下1-batch会被浪费
    // 默认为true，允许拆分task从而使得空间利用率最大。
    if (!batchTask.get_allow_split_request()) {
      if (task->batch_size() > batchTask.get_rem_size() &&
          !batchTask.get_overrun()) {
        break;
      }
    }

    // combine_task_valid负责判断是否能够合并
    // 除最外层的shape外，内层shape应一致或者允许Padding才能合并。
    // 否则跳出循环,放入下一个batchTask中。
    // 以此保证batch.append_task(task)中的task的内层shape相同。

    // 对于Shape[0] = 1 而!=batch的情况，因为合并时，取其中一个的值
    // 所以要求该feedvar必须相等，才能合并。
    // 否则跳出循环,放入下一个batchTask中。
    // 目前没有PaddleTensor和PaddleBuff没有重载==，所以只能比较内存.
    if (previous_task != nullptr) {
      if (task->combine_task_valid(previous_task) == 0) {
        break;
      }
    }

    if (batchTask.padding(task) != 2) {
      break;
    }
    ++padding_task_count;
    size_t rem = batchTask.append_task(task);
    previous_task = task;
    if (task->rem <= 0) {
      _task_queue.pop_front();
    }
    if (rem <= 0) break;
  }

  if (padding_task_count > 1) {
    LOG(INFO) << "Hit auto padding, merge " << padding_task_count
              << " tasks into 1 batch.";
  }
  LOG(INFO) << "Number of tasks remaining in _task_queue is "
            << _task_queue.size();
  return true;
}

// this function is accessed by multi thread.
// move_task_to_batch have add lock inside the function.
// Packaging 1 TaskT as 1 or Several TaskMeta.
// TaskT is from the SingleTon TaskExecutor`s _task_queue
// although TaskMeta is a local variable, but several TaskMeta may points to
// the same TaskT which is get from the SingleTon TaskExecutor`s _task_queue.
// put TaskMeta to the local variable BatchTasks<TaskT> batchTask.

// batchTask.merge_tasks() and batchTask.notify_tasks() has no lock.
// BatchTasks<TaskT> batchTask itself is a local variable, it`s thread safe.
// If batchTask.merge_tasks() and batchTask.notify_tasks() do something to
// TaskMeta
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

    // move_task_to_batch() take the original task from the `_task_queue`
    // put the original task into its own Vector<taskmeta>
    // the capacity of its own Vector<taskmeta> is decided by `_batch_size` or
    // `_overrun`

    // merge_tasks() move the imput-data into `_batch_in` from its own
    // Vector<taskmeta>.
    // because the predictor`s input is the `_batch_in`

    // notify_tasks() move the output-data into every single taskmeta from
    // `_batch_out`.
    // because the predictor`s output is the `_batch_out`
    BatchTasks<TaskT> batchTask(_batch_size, _overrun, _allow_split_request);
    if (move_task_to_batch(batchTask)) {
      batchTask.merge_tasks();
      _fn(&batchTask.in(), &batchTask.out());
      batchTask.notify_tasks();
    }
  }

  return 0;
}

template <typename InItemT, typename OutItemT>
bool TaskManager<InItemT, OutItemT>::schedule(
    const void* in, void* out, MempoolRegion* memoryPtr) {  // NOLINT
  TaskHandler<TaskT> handler =
      TaskExecutorVector<TaskT>::instance()[_model_index].schedule(
          in, out, memoryPtr);

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
