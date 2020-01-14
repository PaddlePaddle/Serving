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

#include <errno.h>
#include <algorithm>
#include <deque>
#include <vector>
#include "core/predictor/common/inner_common.h"
#include "core/predictor/framework/infer_data.h"
#include "core/predictor/framework/memory.h"

#include <boost/function.hpp>

namespace im {
namespace bsf {

template <>
struct Task<baidu::paddle_serving::predictor::Tensor,
            baidu::paddle_serving::predictor::Tensor> {
  typedef Task<baidu::paddle_serving::predictor::Tensor,
               baidu::paddle_serving::predictor::Tensor>
      TaskT;
  typedef baidu::paddle_serving::predictor::Tensor Tensor;
  typedef baidu::paddle_serving::predictor::Tensor InType;
  typedef baidu::paddle_serving::predictor::Tensor OutType;
  typedef baidu::paddle_serving::predictor::BatchTensor BatchTensor;
  typedef baidu::paddle_serving::predictor::BatchTensor InArrayT;
  typedef baidu::paddle_serving::predictor::BatchTensor OutArrayT;

  struct Segment {
    Segment(void* p, size_t b, size_t s) : ptr(p), begin(b), size(s) {}
    void* ptr;
    size_t begin;
    size_t size;
  };

  int read_fd;
  int write_fd;

  pid_t owner_tid;

  const InArrayT* in;
  OutArrayT* out;

  size_t rem;
  size_t size;

  butil::atomic<size_t> index;

  const BatchTensor* get(bool is_in) const {
    if (is_in) {
      return in;
    } else {
      return out;
    }
  }

  BatchTensor* get(bool is_in) {
    if (is_in) {
      return const_cast<BatchTensor*>(in);
    } else {
      return out;
    }
  }

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

template <>
class BatchTasks<Task<baidu::paddle_serving::predictor::Tensor,
                      baidu::paddle_serving::predictor::Tensor>> {
 public:
  typedef baidu::paddle_serving::predictor::Tensor Tensor;
  typedef baidu::paddle_serving::predictor::Tensor InType;
  typedef baidu::paddle_serving::predictor::Tensor OutType;
  typedef baidu::paddle_serving::predictor::DataBuf DataBuf;
  typedef baidu::paddle_serving::predictor::MempoolWrapper MempoolWrapper;

  typedef Task<baidu::paddle_serving::predictor::Tensor,
               baidu::paddle_serving::predictor::Tensor>
      TaskT;
  typedef TaskMeta<TaskT> TaskMetaT;
  typedef TaskT::InArrayT InArrayT;
  typedef TaskT::OutArrayT OutArrayT;

  explicit BatchTasks(size_t batch_size, bool batch_align = false)
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

  static bool check_valid(const InArrayT& in,
                          OutArrayT& out,  // NOLINT
                          bool align) {    // NOLINT
    if (align) {
      if (out.count() <= 0 || out.size() <= 0) {
        LOG(ERROR) << "Out tensor is empty, when aligned";
        return false;
      }

      if (out.size() != in.size()) {
        LOG(ERROR) << "In/Out tensor size not eq: " << out.size()
                   << "!=" << in.size();
        return false;
      }

      for (size_t fi = 0, shape0 = 0; fi < out.count(); ++fi) {
        if (!out[fi].valid()) {
          LOG(ERROR) << "Out[" << fi << "] tensor not valid";
          return false;
        }

        if (out.size() != out[fi].shape0()) {
          LOG(ERROR) << "Shape0 not consistency, " << out.size()
                     << "!=" << out[fi].shape0() << ", " << fi;
          return false;
        }
      }
    }

    return true;
  }

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

  void merge_tasks() {
    merge_input();
    merge_output();
  }

  void merge_input() {
    if (_tasks.size() <= 0 || _tasks[0].task->in->count() <= 0) {
      return;
    }

    if (_tasks.size() == 1 && !_batch_align) {
      TaskMetaT& tm = _tasks[0];
      _batch_in = *(tm.task->in);
      return;
    }

    merge_tensor(true);
  }

  void merge_output() {
    if (_batch_align) {
      if (_tasks.size() <= 0 || _tasks[0].task->out->count() <= 0) {
        return;
      }
    }

    if (_tasks.size() <= 0 || _tasks[0].task->out->count() <= 0) {
      return;
    }

    TaskMetaT& tm = _tasks[0];
    if (_tasks.size() == 1 && !_batch_align) {
      _batch_out = *(tm.task->out);
      return;
    }

    if (tm.task->out->size() <= 0) {
      // shape is empty
      _batch_out = *(tm.task->out);
      return;
    }

    if ((*tm.task->out)[0].data.data() == 0 ||
        (*tm.task->out)[0].data.size() == 0) {
      _batch_out = *(tm.task->out);
      return;
    }

    merge_tensor(false);
  }

  void merge_tensor(bool is_in) {
    // accumulate batch size from fetched tasks
    size_t batch_size = 0;
    for (size_t ti = 0; ti < _tasks.size(); ++ti) {
      TaskMetaT& tm = _tasks[ti];
      size_t add = tm.end - tm.begin;
      batch_size += add;
    }

    // merge all instanses in each tensor data
    size_t tensor_count = _tasks[0].task->get(is_in)->count();
    for (size_t fi = 0; fi < tensor_count; ++fi) {
      const Tensor& head = (*(_tasks[0].task->get(is_in)))[fi];
      Tensor batch_tensor;
      batch_tensor.name = head.name;
      batch_tensor.type = head.type;
      batch_tensor.shape.push_back(batch_size);

      size_t ins_ele_count = 1;
      for (size_t si = 1; si < head.shape.size(); ++si) {
        batch_tensor.shape.push_back(head.shape[si]);
        ins_ele_count *= head.shape[si];
      }

      size_t tensor_ele_count = ins_ele_count * batch_size;
      size_t ins_byte = ins_ele_count * head.ele_byte();

      size_t tensor_byte = tensor_ele_count * head.ele_byte();
      void* data_buf = MempoolWrapper::instance().malloc(tensor_byte);
      if (!data_buf) {
        LOG(ERROR) << "Malloc failed, size: " << tensor_byte;
        return;
      }

      size_t data_byte = 0;
      for (size_t ti = 0; ti < _tasks.size(); ++ti) {
        TaskMetaT& tm = _tasks[ti];
        size_t acc_byte = ins_byte * (tm.end - tm.begin);
        if (data_byte + acc_byte > tensor_byte) {
          LOG(ERROR) << "Invalid bytes: " << data_byte << " + " << acc_byte
                     << " >= " << tensor_byte;
          return;
        }

        const Tensor& tensor = (*(tm.task->get(is_in)))[fi];
        memcpy(
            reinterpret_cast<char*>(data_buf) + data_byte,
            reinterpret_cast<char*>(tensor.data.data()) + tm.begin * ins_byte,
            acc_byte);
        data_byte += acc_byte;
      }

      if (data_byte != tensor_byte) {
        LOG(ERROR) << "Invalid tensor byte: " << data_byte
                   << " != " << tensor_byte;
        return;
      }

      batch_tensor.data =
          DataBuf(reinterpret_cast<char*>(data_buf), tensor_byte);
      if (is_in) {
        _batch_in.push_back(batch_tensor);
      } else {
        _batch_out.push_back(batch_tensor);
      }
    }

    LOG(INFO) << "merge input(" << is_in << ") samples: " << batch_size
              << " from " << _tasks.size() << " pvs";
  }

  void notify_tasks() {
    if (_batch_out.size() != _batch_in.size()) {
      LOG(ERROR) << "batch size not consistency: " << _batch_out.size()
                 << " != " << _batch_in.size();
      return;
    }

    size_t tensor_count = _batch_out.count();
    size_t batch_size = _batch_out.size();
    for (size_t fi = 0; fi < tensor_count; ++fi) {
      const Tensor& tensor = _batch_out[fi];
      size_t ins_byte = tensor.ele_byte();
      for (size_t si = 1; si < tensor.shape.size(); ++si) {
        ins_byte *= tensor.shape[si];
      }

      for (size_t ti = 0, bi = 0, add = 0; ti < _tasks.size();
           ++ti, bi += add) {
        OutArrayT* dst = _tasks[ti].task->out;
        add = _tasks[ti].end - _tasks[ti].begin;
        size_t offset_src = ins_byte * bi;
        size_t add_byte = add * ins_byte;

        if (_batch_align) {  // merge all batchs
          size_t offset_dst = ins_byte * _tasks[ti].begin;
          void* ptr = const_cast<void*>((*dst)[fi].data.data());
          memcpy(
              reinterpret_cast<char*>(ptr) + offset_dst,
              reinterpret_cast<char*>(_batch_out[fi].data.data()) + offset_src,
              add_byte);
        } else {  // overwrite
          if (dst->count() <= 0) {
            dst->push_back(_batch_out[fi]);
          } else {
            (*dst)[fi] = _batch_out[fi];
          }

          (*dst)[fi].shape[0] = add;
          (*dst)[fi].data = DataBuf(
              reinterpret_cast<char*>(_batch_out[fi].data.data()) + offset_src,
              add_byte);
        }
      }
    }

    for (size_t ti = 0; ti < _tasks.size(); ++ti) {
      TaskT* task = _tasks[ti].task;
      size_t begin = _tasks[ti].begin;
      size_t end = _tasks[ti].end;
      size_t add = end - begin;

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
  InArrayT _batch_in;
  OutArrayT _batch_out;
  size_t _batch_size;
  size_t _rem_size;
  bool _batch_align;
};

}  // namespace bsf
}  // namespace im
