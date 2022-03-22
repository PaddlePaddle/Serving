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
#include <cstring>
#include <functional>
#include <list>
#include <numeric>
#include <set>
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
static const size_t ABSOLUTE_ERROR = 1024;
static const float RELATIVE_ERROR = 0.5;
typedef baidu::paddle_serving::predictor::MempoolWrapper MempoolWrapper;
typedef baidu::paddle_serving::predictor::MempoolRegion MempoolRegion;

// InItemT is paddle::PaddleTensor
// InVectorT std::vector<paddle::PaddleTensor>
// InVectorT means different feedvar, but not batch.
// Batch is already inside the  paddle::PaddleTensor.

// size_t `rem` records how many batch have not been put in BatchTasks.
// `rem` don`t need to be atomic, cause the operation `put` is synchronous.
// actually, the reason is that lock have been added outside the operation
// `put`.
template <typename TaskT>
class BatchTasks;
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
  typedef baidu::paddle_serving::predictor::MempoolWrapper MempoolWrapper;

  int read_fd;
  int write_fd;
  pid_t owner_tid;
  const InVectorT* inVectorT_ptr;
  OutVectorT* outVectorT_ptr;
  size_t rem;
  size_t total_feed_batch;
  std::set<size_t> set_feed_lod_index;
  std::set<size_t> set_feed_nobatch_index;
  std::vector<size_t> vector_fetch_lod_index;
  std::set<size_t> set_fetch_nobatch_index;
  butil::atomic<size_t> index;
  size_t taskmeta_num;
  size_t total_taskmeta_num;
  THREAD_MUTEX_T task_mut;
  bool fetch_init;
  // taskmeta_num * set_feed_lod_index.size()
  std::vector<OutVectorT> outLodTensorVector;
  MempoolRegion* memoryPtr;

  Task() {
    read_fd = -1;
    write_fd = -1;
    owner_tid = -1;
    inVectorT_ptr = NULL;
    outVectorT_ptr = NULL;
    set_feed_lod_index.clear();
    set_feed_nobatch_index.clear();
    vector_fetch_lod_index.clear();
    set_fetch_nobatch_index.clear();
    rem = -1;
    total_feed_batch = 0;
    taskmeta_num = 0;
    index.store(0, butil::memory_order_relaxed);
    THREAD_MUTEX_INIT(&task_mut, NULL);
    fetch_init = false;
    total_taskmeta_num = 1;
    outLodTensorVector.clear();
  }
  ~Task() {
    read_fd = -1;
    write_fd = -1;
    owner_tid = -1;
    inVectorT_ptr = NULL;
    outVectorT_ptr = NULL;
    set_feed_lod_index.clear();
    set_feed_nobatch_index.clear();
    vector_fetch_lod_index.clear();
    set_fetch_nobatch_index.clear();
    rem = -1;
    total_feed_batch = 0;
    taskmeta_num = 0;
    total_taskmeta_num = 1;
    index.store(0, butil::memory_order_relaxed);
    THREAD_MUTEX_DESTROY(&task_mut);
    fetch_init = false;
    outLodTensorVector.clear();
  }

  void clear() {
    read_fd = -1;
    write_fd = -1;
    owner_tid = -1;
    inVectorT_ptr = NULL;
    outVectorT_ptr = NULL;
    set_feed_lod_index.clear();
    set_feed_nobatch_index.clear();
    vector_fetch_lod_index.clear();
    set_fetch_nobatch_index.clear();
    rem = -1;
    total_feed_batch = 0;
    taskmeta_num = 0;
    total_taskmeta_num = 1;
    index.store(0, butil::memory_order_relaxed);
    THREAD_MUTEX_INIT(&task_mut, NULL);
    fetch_init = false;
    outLodTensorVector.clear();
  }

  bool check_feedvar_valid(size_t feedvar_index) {
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

  int combine_task_valid(Task* other_task) {
    // 除最外层的shape外，内层shape应一致或者允许Padding才能合并。
    // 否则跳出循环,放入下一个batchTask中。
    // 当内层shape不一致时，此时先不判断是否Padding，在batchTask层判断，返回2。
    // 以此保证batch.append_task(task)中的task的内层shape相同。

    // return 0 表示Shape[0] = 1
    // 而!=batch的情况，两个Task中的值不同，此时不能合并。
    // return 1 表示Shape维度完全一致，直接合并即可。
    // return 2 表示Shape维度不完全一致，还需要进一步的判断，是否合并。
    if (other_task->feedvar_shape_nobatch() != feedvar_shape_nobatch()) {
      return 2;
    }

    // 对于Shape[0] = 1 而!=batch的情况，因为合并时，取其中一个的值
    // 所以要求该feedvar必须相等，才能合并。
    // 目前没有PaddleTensor和PaddleBuff没有重载==，所以只能比较内存.
    for (size_t feedvar_index = 0;
         feedvar_index < set_feed_nobatch_index.size();
         ++feedvar_index) {
      int result =
          std::memcmp((*inVectorT_ptr)[feedvar_index].data.data(),
                      (*(other_task->inVectorT_ptr))[feedvar_index].data.data(),
                      (*inVectorT_ptr)[feedvar_index].data.length());
      if (result != 0) return 0;
    }
    return 1;
  }

  size_t feedvar_batch_size(size_t feedvar_index) {
    if (!check_feedvar_valid(feedvar_index)) {
      return 0;
    }
    // if lod, 'lod[0].size()-1' is batch.
    // for PaddleTensor lod is vector<vector<size_t>>, so lod[0] is real lod.
    // for example, lod = [0,3,4,6], shape = [6,340,340], batch is 3 actually.
    // for lod, the batch < shape[0].
    if ((*inVectorT_ptr)[feedvar_index].lod.size() > 0 &&
        (*inVectorT_ptr)[feedvar_index].lod[0].size() > 0) {
      return (*inVectorT_ptr)[feedvar_index].lod[0].size() - 1;
    }
    // if not lod, the first dimension of data `PaddleTensor.shape[0]` is batch.
    return (*inVectorT_ptr)[feedvar_index].shape[0];
  }

  size_t feedvar_element_bytesize(size_t feedvar_index) {
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
  size_t feedvar_element_num(size_t feedvar_index) {
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
    for (size_t i = 1; i < (*inVectorT_ptr)[feedvar_index].shape.size(); ++i) {
      element_num *= (*inVectorT_ptr)[feedvar_index].shape[i];
    }
    return element_num;
  }

  size_t feedvar_bytesize(size_t feedvar_index) {
    return feedvar_element_num(feedvar_index) *
           feedvar_element_bytesize(feedvar_index);
  }

  ShapeVector feedvar_shape_nobatch(size_t feedvar_index) {
    if (!check_feedvar_valid(feedvar_index)) {
      return ShapeVector();
    }
    return ShapeVector{(*inVectorT_ptr)[feedvar_index].shape.begin() + 1,
                       (*inVectorT_ptr)[feedvar_index].shape.end()};
  }

  VectorOfShapeVector feedvar_shape_nobatch() {
    VectorOfShapeVector vector_of_feedvar_shape_nobatch;
    for (size_t feedvar_index = 0; feedvar_index < inVectorT_ptr->size();
         ++feedvar_index) {
      vector_of_feedvar_shape_nobatch.push_back(
          feedvar_shape_nobatch(feedvar_index));
    }
    return vector_of_feedvar_shape_nobatch;
  }

  // For each feedvar, batch should be 1 or batch_size.
  // if feedvar-1: batch_size = 1 (always not batch).
  // feedvar-2: batch_size = n,  batch = n.
  // this function is not thread safe. only called when task is creating.
  bool task_init() {
    total_feed_batch = feedvar_batch_size(0);
    // which means error.
    if (total_feed_batch <= 0) return false;

    for (size_t feedvar_index = 0; feedvar_index < inVectorT_ptr->size();
         ++feedvar_index) {
      // TODO(HexToString): Distinguish between nobatch and batch =
      // 1(By:HexToString)
      // 当数据中feedvar-1: 带batch,且batch =1，shape[0] = 1
      // feedvar-2:不带batch，由于不带batch导致shape[0] =1
      // 此时，无法分辨是否是天然nobatch，此时set_feed_nobatch_index会漏掉
      // 后续希望在其他地方能够区分两者。
      if (feedvar_batch_size(feedvar_index) != total_feed_batch) {
        // which means error.
        if (feedvar_batch_size(feedvar_index) != 1 && total_feed_batch != 1) {
          return false;
        } else if (feedvar_batch_size(feedvar_index) != 1 &&
                   total_feed_batch == 1) {
          for (int temp = 0; temp < feedvar_index; ++temp) {
            set_feed_nobatch_index.insert(temp);
          }
          total_feed_batch = feedvar_batch_size(feedvar_index);
        } else {
          set_feed_nobatch_index.insert(feedvar_index);
        }
      }
      // 将lod feedvar index加入到vector中。
      if ((*inVectorT_ptr)[feedvar_index].lod.size() > 0 &&
          (*inVectorT_ptr)[feedvar_index].lod[0].size() > 0) {
        set_feed_lod_index.insert(feedvar_index);
      }
    }
    return true;
  }

  size_t batch_size() { return total_feed_batch; }

  // start_batch range is 0~batch_size, end_batch range is 1~batch_size
  // start_batch should not be included, end_batch > start_batch
  // return is (start_batch, end_batch] = [start_batch+1,end_batch]
  // for not lod, shape0_index = [(start_batch+1)-1,end_batch-1] =
  // [start_batch,end_batch-1] = [start_batch,end_batch)
  // for lod, shape0_index = [lod[start_batch],lod[end_batch]-1] =
  // [lod[start_batch],lod[end_batch])
  // for nobatch, shape0_index = [0,1)
  // 对于调用者，拿到shape0_index后，for(size_t myindex =shape0_index[0];
  // myindex <shape0_index[1];myindex++)即可.

  // 原始lod= [0,3,4,6] 取的batch为(start_batch = 1,end_batch =
  // 3]，即取batch=2,3.
  // 此时lod=[3,4,6]，处理后得到[1,3]
  // 这样处理后，合并lod比较方便，直接加上上一个lod的结尾的值即可。
  std::vector<std::vector<size_t>> get_feature_by_batch(size_t feedvar_index,
                                                        size_t start_batch,
                                                        size_t end_batch) {
    std::vector<std::vector<size_t>> feature_vector;
    // feature_vector是双层vector,这么设计是由于一个遍历即可处理所有的特征。
    // feature_vector[0]是由shape0_index的范围值组成的vector,包含两个元素最小和最大值。
    // feature_vector[1]是由lod组成的vector，包含指定batch的lod信息.
    // feature_vector[2]是由单个元素的组成的vector，元素值为1表示是nobatch的feedvar。
    // feature_vector[3]是2维lod组成的vector,包含指定batch的2-level lod。
    // 之所以把二维lod
    // 加入到feature_vector[3]，是为了兼容原有代码，尽可能小的改动。

    // if 为 nobatch feedvar情况。
    // else if 为带lod的feedvar情况。
    // else为不带lod 普通feedvar情况。
    if (set_feed_nobatch_index.size() > 0 &&
        set_feed_nobatch_index.find(feedvar_index) !=
            set_feed_nobatch_index.end()) {
      feature_vector = {{0, 1}, {}, {1}};
    } else if (set_feed_lod_index.size() > 0 &&
               set_feed_lod_index.find(feedvar_index) !=
                   set_feed_lod_index.end()) {
      int lod_size = (*inVectorT_ptr)[feedvar_index].lod.size();
      std::vector<size_t> feed_lod_vector(end_batch - start_batch);
      for (size_t lod_index = start_batch + 1, vector_index = 0;
           lod_index < end_batch + 1;
           ++lod_index, ++vector_index) {
        feed_lod_vector[vector_index] =
            (*inVectorT_ptr)[feedvar_index].lod[0][lod_index] -
            (*inVectorT_ptr)[feedvar_index].lod[0][start_batch];
      }
      if (lod_size == 1) {
        size_t shape0_start =
            (*inVectorT_ptr)[feedvar_index].lod[0][start_batch];
        size_t shape0_end = (*inVectorT_ptr)[feedvar_index].lod[0][end_batch];
        feature_vector = {{shape0_start, shape0_end}, feed_lod_vector};
      } else if (lod_size == 2) {
        size_t level2_lod_start_index =
            (*inVectorT_ptr)[feedvar_index].lod[0][start_batch];
        size_t level2_lod_end_index =
            (*inVectorT_ptr)[feedvar_index].lod[0][end_batch];
        int level2_lod_size = level2_lod_end_index - level2_lod_start_index;
        std::vector<size_t> feed_2level_lod_vector(level2_lod_size);
        for (size_t lod2_index = level2_lod_start_index + 1, vector_index = 0;
             lod2_index < level2_lod_end_index + 1;
             ++vector_index, ++lod2_index) {
          feed_2level_lod_vector[vector_index] =
              (*inVectorT_ptr)[feedvar_index].lod[1][lod2_index] -
              (*inVectorT_ptr)[feedvar_index].lod[1][level2_lod_start_index];
        }
        size_t shape0_start =
            (*inVectorT_ptr)[feedvar_index].lod[1][level2_lod_start_index];
        size_t shape0_end =
            (*inVectorT_ptr)[feedvar_index].lod[1][level2_lod_end_index];
        feature_vector = {{shape0_start, shape0_end},
                          feed_lod_vector,
                          {},
                          feed_2level_lod_vector};
      }

      // feature_vector.push_back(feed_lod_vector);
    } else {
      feature_vector = {{start_batch, end_batch}};
    }
    return feature_vector;
  }

  bool combine_taskmeta() {
    // 只有含有lod类型的fetch输出，且task被拆分为多个taskmeta的情况
    // 才需要将数据从outLodTensorVector搬运到outVectorT_ptr
    if (vector_fetch_lod_index.size() > 0 && total_taskmeta_num > 1) {
      for (size_t index = 0; index < vector_fetch_lod_index.size(); ++index) {
        size_t data_length = 0;
        size_t lod_length = 0;
        size_t lod2_length = 0;
        size_t total_shape0 = 0;
        size_t once_lod0_length = 0;
        int lod_size = 1;
        size_t feedvar_index = vector_fetch_lod_index[index];
        // 由于PaddleTensor的resize实现，是每次都会清空，所以必须先统计总长度。
        for (size_t taskmeta_index = 0; taskmeta_index < total_taskmeta_num;
             ++taskmeta_index) {
          lod_size = outLodTensorVector[taskmeta_index][index].lod.size();
          data_length +=
              outLodTensorVector[taskmeta_index][index].data.length();
          once_lod0_length =
              outLodTensorVector[taskmeta_index][index].lod[0].size();
          lod_length += once_lod0_length;
          total_shape0 += outLodTensorVector[taskmeta_index][index].shape[0];
          if (lod_size == 2) {
            lod2_length += outLodTensorVector[taskmeta_index][index]
                               .lod[0][once_lod0_length - 1];
          }
        }
        // 一次性扩容PaddleTensor中的data和lod
        paddle::PaddleTensor& fetchVarTensor = (*outVectorT_ptr)[feedvar_index];
        fetchVarTensor.shape[0] = total_shape0;
        void* databuf_data =
            MempoolWrapper::instance().malloc(data_length, memoryPtr);
        paddle::PaddleBuf paddleBuf(databuf_data, data_length);
        fetchVarTensor.data = paddleBuf;

        // fetchVarTensor.data.Resize(data_length);
        // task中的lod补0
        if (fetchVarTensor.lod.size() <= 0) {
          fetchVarTensor.lod.push_back({0});
        } else if (fetchVarTensor.lod[0].size() <= 0) {
          fetchVarTensor.lod[0].push_back(0);
        }
        fetchVarTensor.lod[0].resize(lod_length + 1, 0);
        if (lod_size == 2) {
          if (fetchVarTensor.lod.size() <= 1) {
            fetchVarTensor.lod.push_back({0});
          } else if (fetchVarTensor.lod[1].size() <= 0) {
            fetchVarTensor.lod[1].push_back(0);
          }
          fetchVarTensor.lod[1].resize(lod2_length + 1, 0);
        }

        //
        size_t data_length_offset = 0;
        size_t lod_length_offset = 0;
        size_t lod2_length_offset = 0;
        size_t once_data_length = 0;
        size_t once_lod_length = 0;
        size_t once_2lod_length = 0;
        for (size_t taskmeta_index = 0; taskmeta_index < total_taskmeta_num;
             ++taskmeta_index) {
          // process data
          void* dst_ptr = fetchVarTensor.data.data() + data_length_offset;
          void* source_ptr =
              outLodTensorVector[taskmeta_index][index].data.data();
          once_data_length =
              outLodTensorVector[taskmeta_index][index].data.length();
          memcpy(dst_ptr, source_ptr, once_data_length);
          data_length_offset += once_data_length;
          // process lod
          size_t last_lod_value = fetchVarTensor.lod[0][lod_length_offset];
          once_lod_length =
              outLodTensorVector[taskmeta_index][index].lod[0].size();
          for (size_t once_index = 0; once_index < once_lod_length;
               ++once_index) {
            fetchVarTensor.lod[0][lod_length_offset + 1] =
                last_lod_value +
                outLodTensorVector[taskmeta_index][index].lod[0][once_index];
            lod_length_offset++;
          }
          if (lod_size == 2) {
            size_t last_2lod_value = fetchVarTensor.lod[1][lod2_length_offset];
            once_2lod_length =
                outLodTensorVector[taskmeta_index][index].lod[1].size();
            for (size_t once_index = 0; once_index < once_2lod_length;
                 ++once_index) {
              fetchVarTensor.lod[1][lod2_length_offset + 1] =
                  last_2lod_value +
                  outLodTensorVector[taskmeta_index][index].lod[1][once_index];
              lod2_length_offset ++;
            }
          }
        }
      }
    }
    return true;
  }

  bool task_fetch_init(BatchTasks<TaskT>& batchTask);
  bool task_fetch_create(BatchTasks<TaskT>& batchTask);
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
// `_batch_out`(which is in BatchTasks) and `outVectorT_ptr`(which is in Task).
// especially when 1 Task be divided into several TaskMeta and be put into
// several different BatchTasks.

// begin、add、end means batch, not shape[0].
// if not lod, batch == shape[0]. if lod, batch != shape[0]
// for example, lod = [0,3,4,6], shape = [6,340,340]
// there is 3 batch actually, add = 3, but shape[0] = 6.
template <typename TaskT>
struct TaskMeta {
  TaskMeta(TaskT* ptr, size_t start, size_t add, size_t taskmeta_index)
      : task(ptr),
        begin(start),
        end(start + add),
        taskmeta_index(taskmeta_index) {
    feedvar_num = ptr->inVectorT_ptr->size();
    for (size_t feedvar_index = 0; feedvar_index < feedvar_num;
         ++feedvar_index) {
      std::vector<std::vector<size_t>> feature =
          ptr->get_feature_by_batch(feedvar_index, start, start + add);
      feed_shape0_range.push_back(feature[0]);
      feedvar_type.push_back(feature.size());
      if (feature.size() == 1) {
        feed_lod_vector.push_back({});
        feed_2level_lod_vector.push_back({});
      } else if (feature.size() == 2) {
        feed_lod_vector.push_back(feature[1]);
        feed_2level_lod_vector.push_back({});
      } else if (feature.size() == 3) {
        feed_lod_vector.push_back({});
        feed_2level_lod_vector.push_back({});
      } else if (feature.size() == 4) {
        feed_lod_vector.push_back(feature[1]);
        feed_2level_lod_vector.push_back(feature[3]);
      }
    }
  }

  TaskT* task;
  size_t begin;
  size_t end;
  size_t feedvar_num;
  size_t taskmeta_index;
  std::vector<std::vector<size_t>> feed_shape0_range;
  std::vector<std::vector<size_t>> feed_lod_vector;
  std::vector<std::vector<size_t>> feed_2level_lod_vector;
  std::vector<size_t> feedvar_type;
};

// each TaskT is already include batch in itself
// BatchTasks need to combine several `small TaskMeta` into a new `big TaskT`.
// The only difference between the `big TaskT` and `small TaskT` is that
// the TaskT.inVectorT_ptr->[feedvar_index].shape[0] is different
// `big TaskT`.inVectorT_ptr->[feedvar_index].shape[0] is actually batch_size .
template <typename TaskT>
class BatchTasks {
 public:
  typedef typename TaskT::InType InType;
  typedef typename TaskT::OutType OutType;
  typedef TaskMeta<TaskT> TaskMetaT;
  typedef std::vector<int> ShapeVector;
  typedef std::vector<ShapeVector> VectorOfShapeVector;
  typedef std::vector<size_t> LodVector;
  typedef std::vector<LodVector> PaddleTensorLod;
  friend TaskT;

  explicit BatchTasks(size_t batch_size,
                      bool overrun = false,
                      bool allow_split_request = true,
                      bool auto_padding = true,
                      int padding_value = 0)
      : _batch_size(batch_size),
        _rem_size(batch_size),
        _overrun(overrun),
        _allow_split_request(allow_split_request),
        _auto_padding(auto_padding),
        _padding_value(padding_value) {
    _batch_in.clear();
    _batch_in_offset.clear();
    _total_shape0_batch_in.clear();
    _total_feed_batch = 0;
    _batch_in_lod.clear();

    _batch_out.clear();
    _batch_out_offset.clear();
    _total_fetch_batch = 0;
    _taskmeta_vector.clear();
    set_fetch_nobatch_index.clear();
    vector_fetch_lod_index.clear();
  }

  ~BatchTasks() {
    _batch_in.clear();
    _batch_in_offset.clear();
    _total_shape0_batch_in.clear();
    _total_feed_batch = 0;
    _batch_in_lod.clear();

    _batch_out.clear();
    _batch_out_offset.clear();
    _total_fetch_batch = 0;
    _taskmeta_vector.clear();
    set_fetch_nobatch_index.clear();
    vector_fetch_lod_index.clear();
  }

  // return 0
  // 表示feedvar数量都不一样，或者，每个feedvar的shape维度都不同，此时不能合并batch。
  // return 1 表示合并batch不划算。
  // return 2 表示合并batch划算。
  int padding(TaskT* task) {
    const VectorOfShapeVector& task_vector_shape =
        task->feedvar_shape_nobatch();
    int return_value = 2;

    // 当batchTask中为空时，第一次加入Task，此时则BatchTask中即为第一个Task中的Shape.
    if (vector_of_max_shape.size() == 0) {
      vector_of_max_shape = task_vector_shape;
      return 2;
    }

    if (vector_of_max_shape.size() != task_vector_shape.size()) {
      return 0;
    }

    // 当两个Shape完全相同时，无须更新，无须计算，无须Padding。
    if (vector_of_max_shape == task_vector_shape) {
      return 2;
    }

    std::vector<size_t> multiplies_1(vector_of_max_shape.size());
    std::vector<size_t> multiplies_2(vector_of_max_shape.size());
    std::vector<size_t> temp_multiplies(vector_of_max_shape.size());
    VectorOfShapeVector temp_vector_max_shape(vector_of_max_shape.size());
    for (size_t i = 0; i < vector_of_max_shape.size(); ++i) {
      if (vector_of_max_shape[i].size() != task_vector_shape[i].size())
        return 0;
      for (size_t j = 0; j < vector_of_max_shape[i].size(); ++j) {
        temp_vector_max_shape[i].push_back(
            std::max(vector_of_max_shape[i][j], task_vector_shape[i][j]));
      }
      temp_multiplies[i] = std::accumulate(temp_vector_max_shape[i].begin(),
                                           temp_vector_max_shape[i].end(),
                                           1,
                                           std::multiplies<size_t>());
      multiplies_1[i] = std::accumulate(vector_of_max_shape[i].begin(),
                                        vector_of_max_shape[i].end(),
                                        1,
                                        std::multiplies<size_t>());
      multiplies_2[i] = std::accumulate(task_vector_shape[i].begin(),
                                        task_vector_shape[i].end(),
                                        1,
                                        std::multiplies<size_t>());
      if ((labs(temp_multiplies[i] - multiplies_1[i]) <= ABSOLUTE_ERROR &&
           labs(temp_multiplies[i] - multiplies_2[i]) <= ABSOLUTE_ERROR) ||
          (temp_multiplies[i] / multiplies_1[i] >= RELATIVE_ERROR &&
           temp_multiplies[i] / multiplies_2[i] >= RELATIVE_ERROR)) {
        continue;
      } else {
        return_value = 1;
      }
    }

    // 当合并batch时，需要更新BatchTask中的最大Shape
    // 此时，整个BatchTask到最后合并多个Task时，需要Padding
    if (return_value == 2) {
      vector_of_max_shape = temp_vector_max_shape;
    }
    return return_value;
  }

  // synchronized operation
  // because Upper level callers of this function have already locked.
  // 能进到此函数的task都是同类task，在该函数之前已保证了这点。
  size_t append_task(TaskT* task) {
    size_t add = std::min(task->rem, _rem_size);
    // when _overrun == true, it means always take a whole task as TaskMeta
    // we can temporary breakthrough the limit of BatchTask`s capacity
    // BatchTask`s capacity is _batch_size or _rem_size
    if (_overrun) {
      add = task->rem;
    }
    int start_index = task->batch_size() - task->rem;
    TaskMetaT tm(task, start_index, add, task->taskmeta_num);
    task->rem -= add;
    _rem_size -= add;
    if (task->taskmeta_num == 0) {
      task->total_taskmeta_num =
          1 + (task->rem + _batch_size - 1) / _batch_size;
    }
    task->taskmeta_num += 1;
    _taskmeta_vector.push_back(tm);
    if (_batch_in_offset.size() == 0) {
      _batch_in_offset.resize(tm.feedvar_num, 0);
    }
    if (_total_shape0_batch_in.size() == 0) {
      _total_shape0_batch_in.resize(tm.feedvar_num, 0);
    }
    if (_batch_in_lod.size() == 0) {
      PaddleTensorLod null_lod;
      _batch_in_lod.resize(tm.feedvar_num, null_lod);
    }
    _total_feed_batch += add;
    for (size_t feedvar_index = 0; feedvar_index < tm.feedvar_num;
         ++feedvar_index) {
      if (tm.feedvar_type[feedvar_index] == 1) {
        // 普通的非lod feedvar
        // 累计计算shape0的累加值，为后面初始化PaddleTensor做准备。
        _total_shape0_batch_in[feedvar_index] +=
            tm.feed_shape0_range[feedvar_index][1] -
            tm.feed_shape0_range[feedvar_index][0];
      } else if (tm.feedvar_type[feedvar_index] == 3) {
        // tm.feedvar_type[feedvar_index] == 3
        // nobatch类型的feedvar.
        // 此时不累加，且值应为1
        _total_shape0_batch_in[feedvar_index] =
            tm.feed_shape0_range[feedvar_index][1] -
            tm.feed_shape0_range[feedvar_index][0];
      } else {
        // lod类型的feedvar 可能是1维lod 也可能是2维lod
        // 累计计算shape0的累加值，为后面初始化PaddleTensor做准备。
        _total_shape0_batch_in[feedvar_index] +=
            tm.feed_shape0_range[feedvar_index][1] -
            tm.feed_shape0_range[feedvar_index][0];
        // 在Lod最前面加0
        if (_batch_in_lod[feedvar_index].size() <= 0) {
          _batch_in_lod[feedvar_index].push_back({0});
        } else if (_batch_in_lod[feedvar_index][0].size() <= 0) {
          _batch_in_lod[feedvar_index][0].push_back(0);
        }
        // 将lod加上前一组lod的结尾最大值，组合Lod
        size_t last_lod_value = _batch_in_lod[feedvar_index][0].back();
        for (size_t lod_index = 0;
             lod_index < tm.feed_lod_vector[feedvar_index].size();
             ++lod_index) {
          _batch_in_lod[feedvar_index][0].push_back(
              last_lod_value + tm.feed_lod_vector[feedvar_index][lod_index]);
        }

        // 2维lod 需要额外处理2维lod信息。
        if (tm.feedvar_type[feedvar_index] == 4) {
          if (_batch_in_lod[feedvar_index].size() <= 1) {
            _batch_in_lod[feedvar_index].push_back({0});
          } else if (_batch_in_lod[feedvar_index][1].size() <= 0) {
            _batch_in_lod[feedvar_index][1].push_back(0);
          }
          size_t last_lod_value = _batch_in_lod[feedvar_index][1].back();
          for (size_t lod_index = 0;
               lod_index < tm.feed_2level_lod_vector[feedvar_index].size();
               ++lod_index) {
            _batch_in_lod[feedvar_index][1].push_back(
                last_lod_value +
                tm.feed_2level_lod_vector[feedvar_index][lod_index]);
          }
        }
      }
    }
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

  // batch.merge_tasks() is thread-safe function
  // cause batch is a local variable and Task is just read, not written.

  void merge_tasks() {
    if (_taskmeta_vector.size() <= 0) {
      return;
    }
    for (size_t ti = 0; ti < _taskmeta_vector.size(); ++ti) {
      TaskMetaT& tm = _taskmeta_vector[ti];

      for (size_t feedvar_index = 0; feedvar_index < tm.feedvar_num;
           ++feedvar_index) {
        const paddle::PaddleTensor& feedVarTensor =
            (*tm.task->inVectorT_ptr)[feedvar_index];
        size_t feedvar_bytesize = tm.task->feedvar_bytesize(feedvar_index);
        const ShapeVector& feedvar_max_shape_vector =
            vector_of_max_shape[feedvar_index];
        size_t feedvar_max_num =
            std::accumulate(feedvar_max_shape_vector.begin(),
                            feedvar_max_shape_vector.end(),
                            1,
                            std::multiplies<size_t>());
        size_t feedvar_element_bytesize =
            tm.task->feedvar_element_bytesize(feedvar_index);
        size_t feedvar_max_bytes = feedvar_element_bytesize * feedvar_max_num;

        if (ti == 0) {
          // Create the entire tensor at once
          paddle::PaddleTensor paddleTensor;
          paddleTensor.dtype = feedVarTensor.dtype;
          paddleTensor.name = feedVarTensor.name;
          paddleTensor.lod = _batch_in_lod[feedvar_index];
          paddleTensor.shape = feedvar_max_shape_vector;
          paddleTensor.shape.insert(paddleTensor.shape.begin(),
                                    _total_shape0_batch_in[feedvar_index]);
          size_t databuf_size =
              feedvar_max_bytes * _total_shape0_batch_in[feedvar_index];
          void* databuf_data = MempoolWrapper::instance().malloc(databuf_size);
          paddle::PaddleBuf paddleBuf(databuf_data, databuf_size);
          paddleTensor.data = paddleBuf;
          _batch_in.push_back(paddleTensor);
        }

        void* dst_ptr = _batch_in[feedvar_index].data.data() +
                        _batch_in_offset[feedvar_index];
        void* source_ptr =
            feedVarTensor.data.data() +
            feedvar_bytesize * tm.feed_shape0_range[feedvar_index][0];
        size_t length =
            feedvar_max_bytes * (tm.feed_shape0_range[feedvar_index][1] -
                                 tm.feed_shape0_range[feedvar_index][0]);

        // 不需要padding，连续内存，则直接memcpy
        // 这里可以直接比较内存是否一样大
        // 在于前面Padding函数中，已经保证了vector_of_max_shape中各个维度都是最大值。
        // 当shape-1 = [8000,20000] shape-2 = [20000,8000]时
        // 此时，vector_of_max_shape中的shape = [20000,20000]
        // 所以feedvar_max_bytes == feedvar_bytesize时，一定是shape完全相同。
        if (feedvar_max_bytes == feedvar_bytesize) {
          memcpy(dst_ptr, source_ptr, length);
        } else {
          memset(dst_ptr, 0, length);
          size_t old_index = 0;
          size_t new_index = 0;

          switch (feedvar_max_shape_vector.size()) {
            case 5:
              for (int i_0 = tm.feed_shape0_range[feedvar_index][0];
                   i_0 < tm.feed_shape0_range[feedvar_index][1];
                   ++i_0) {
                for (int i_1 = 0; i_1 < feedVarTensor.shape[1]; ++i_1) {
                  for (int i_2 = 0; i_2 < feedVarTensor.shape[2]; ++i_2) {
                    for (int i_3 = 0; i_3 < feedVarTensor.shape[3]; ++i_3) {
                      for (int i_4 = 0; i_4 < feedVarTensor.shape[4]; ++i_4) {
                        for (int i_5 = 0; i_5 < feedVarTensor.shape[5]; ++i_5) {
                          old_index = i_0 * feedVarTensor.shape[1] *
                                          feedVarTensor.shape[2] *
                                          feedVarTensor.shape[3] *
                                          feedVarTensor.shape[4] *
                                          feedVarTensor.shape[5] +
                                      i_1 * feedVarTensor.shape[2] *
                                          feedVarTensor.shape[3] *
                                          feedVarTensor.shape[4] *
                                          feedVarTensor.shape[5] +
                                      i_2 * feedVarTensor.shape[3] *
                                          feedVarTensor.shape[4] *
                                          feedVarTensor.shape[5] +
                                      i_3 * feedVarTensor.shape[4] *
                                          feedVarTensor.shape[5] +
                                      i_4 * feedVarTensor.shape[5] + i_5;
                          new_index = i_0 * feedvar_max_shape_vector[0] *
                                          feedvar_max_shape_vector[1] *
                                          feedvar_max_shape_vector[2] *
                                          feedvar_max_shape_vector[3] *
                                          feedvar_max_shape_vector[4] +
                                      i_1 * feedvar_max_shape_vector[1] *
                                          feedvar_max_shape_vector[2] *
                                          feedvar_max_shape_vector[3] *
                                          feedvar_max_shape_vector[4] +
                                      i_2 * feedvar_max_shape_vector[2] *
                                          feedvar_max_shape_vector[3] *
                                          feedvar_max_shape_vector[4] +
                                      i_3 * feedvar_max_shape_vector[3] *
                                          feedvar_max_shape_vector[4] +
                                      i_4 * feedvar_max_shape_vector[4] + i_5;
                          if (feedVarTensor.dtype ==
                              paddle::PaddleDType::INT64) {
                            *((int64_t*)dst_ptr + new_index) =
                                *((int64_t*)source_ptr + old_index);
                          } else if (feedVarTensor.dtype ==
                                     paddle::PaddleDType::FLOAT32) {
                            *((float*)dst_ptr + new_index) =
                                *((float*)source_ptr + old_index);
                          } else if (feedVarTensor.dtype ==
                                     paddle::PaddleDType::INT32) {
                            *((int32_t*)dst_ptr + new_index) =
                                *((int32_t*)source_ptr + old_index);
                          } else if (feedVarTensor.dtype ==
                                     paddle::PaddleDType::UINT8) {
                            *((char*)dst_ptr + new_index) =
                                *((char*)source_ptr + old_index);
                          }
                        }
                      }
                    }
                  }
                }
              }
              break;
            case 4:
              for (int i_0 = tm.feed_shape0_range[feedvar_index][0];
                   i_0 < tm.feed_shape0_range[feedvar_index][1];
                   ++i_0) {
                for (int i_1 = 0; i_1 < feedVarTensor.shape[1]; ++i_1) {
                  for (int i_2 = 0; i_2 < feedVarTensor.shape[2]; ++i_2) {
                    for (int i_3 = 0; i_3 < feedVarTensor.shape[3]; ++i_3) {
                      for (int i_4 = 0; i_4 < feedVarTensor.shape[4]; ++i_4) {
                        old_index = i_0 * feedVarTensor.shape[1] *
                                        feedVarTensor.shape[2] *
                                        feedVarTensor.shape[3] *
                                        feedVarTensor.shape[4] +
                                    i_1 * feedVarTensor.shape[2] *
                                        feedVarTensor.shape[3] *
                                        feedVarTensor.shape[4] +
                                    i_2 * feedVarTensor.shape[3] *
                                        feedVarTensor.shape[4] +
                                    i_3 * feedVarTensor.shape[4] + i_4;
                        new_index = i_0 * feedvar_max_shape_vector[0] *
                                        feedvar_max_shape_vector[1] *
                                        feedvar_max_shape_vector[2] *
                                        feedvar_max_shape_vector[3] +
                                    i_1 * feedvar_max_shape_vector[1] *
                                        feedvar_max_shape_vector[2] *
                                        feedvar_max_shape_vector[3] +
                                    i_2 * feedvar_max_shape_vector[2] *
                                        feedvar_max_shape_vector[3] +
                                    i_3 * feedvar_max_shape_vector[3] + i_4;
                        if (feedVarTensor.dtype == paddle::PaddleDType::INT64) {
                          *((int64_t*)dst_ptr + new_index) =
                              *((int64_t*)source_ptr + old_index);
                        } else if (feedVarTensor.dtype ==
                                   paddle::PaddleDType::FLOAT32) {
                          *((float*)dst_ptr + new_index) =
                              *((float*)source_ptr + old_index);
                        } else if (feedVarTensor.dtype ==
                                   paddle::PaddleDType::INT32) {
                          *((int32_t*)dst_ptr + new_index) =
                              *((int32_t*)source_ptr + old_index);
                        } else if (feedVarTensor.dtype ==
                                   paddle::PaddleDType::UINT8) {
                          *((char*)dst_ptr + new_index) =
                              *((char*)source_ptr + old_index);
                        }
                      }
                    }
                  }
                }
              }
              break;
            case 3:
              for (int i_0 = tm.feed_shape0_range[feedvar_index][0];
                   i_0 < tm.feed_shape0_range[feedvar_index][1];
                   ++i_0) {
                for (int i_1 = 0; i_1 < feedVarTensor.shape[1]; ++i_1) {
                  for (int i_2 = 0; i_2 < feedVarTensor.shape[2]; ++i_2) {
                    for (int i_3 = 0; i_3 < feedVarTensor.shape[3]; ++i_3) {
                      old_index = i_0 * feedVarTensor.shape[1] *
                                      feedVarTensor.shape[2] *
                                      feedVarTensor.shape[3] +
                                  i_1 * feedVarTensor.shape[2] *
                                      feedVarTensor.shape[3] +
                                  i_2 * feedVarTensor.shape[3] + i_3;
                      new_index = i_0 * feedvar_max_shape_vector[0] *
                                      feedvar_max_shape_vector[1] *
                                      feedvar_max_shape_vector[2] +
                                  i_1 * feedvar_max_shape_vector[1] *
                                      feedvar_max_shape_vector[2] +
                                  i_2 * feedvar_max_shape_vector[2] + i_3;
                      if (feedVarTensor.dtype == paddle::PaddleDType::INT64) {
                        *((int64_t*)dst_ptr + new_index) =
                            *((int64_t*)source_ptr + old_index);
                      } else if (feedVarTensor.dtype ==
                                 paddle::PaddleDType::FLOAT32) {
                        *((float*)dst_ptr + new_index) =
                            *((float*)source_ptr + old_index);
                      } else if (feedVarTensor.dtype ==
                                 paddle::PaddleDType::INT32) {
                        *((int32_t*)dst_ptr + new_index) =
                            *((int32_t*)source_ptr + old_index);
                      } else if (feedVarTensor.dtype ==
                                 paddle::PaddleDType::UINT8) {
                        *((char*)dst_ptr + new_index) =
                            *((char*)source_ptr + old_index);
                      }
                    }
                  }
                }
              }
              break;
            case 2:
              for (int i_0 = tm.feed_shape0_range[feedvar_index][0];
                   i_0 < tm.feed_shape0_range[feedvar_index][1];
                   ++i_0) {
                for (int i_1 = 0; i_1 < feedVarTensor.shape[1]; ++i_1) {
                  for (int i_2 = 0; i_2 < feedVarTensor.shape[2]; ++i_2) {
                    old_index =
                        i_0 * feedVarTensor.shape[1] * feedVarTensor.shape[2] +
                        i_1 * feedVarTensor.shape[2] + i_2;
                    new_index = i_0 * feedvar_max_shape_vector[0] *
                                    feedvar_max_shape_vector[1] +
                                i_1 * feedvar_max_shape_vector[1] + i_2;
                    if (feedVarTensor.dtype == paddle::PaddleDType::INT64) {
                      *((int64_t*)dst_ptr + new_index) =
                          *((int64_t*)source_ptr + old_index);
                    } else if (feedVarTensor.dtype ==
                               paddle::PaddleDType::FLOAT32) {
                      *((float*)dst_ptr + new_index) =
                          *((float*)source_ptr + old_index);
                    } else if (feedVarTensor.dtype ==
                               paddle::PaddleDType::INT32) {
                      *((int32_t*)dst_ptr + new_index) =
                          *((int32_t*)source_ptr + old_index);
                    } else if (feedVarTensor.dtype ==
                               paddle::PaddleDType::UINT8) {
                      *((char*)dst_ptr + new_index) =
                          *((char*)source_ptr + old_index);
                    }
                  }
                }
              }
              break;
            case 1:
              for (int i_0 = tm.feed_shape0_range[feedvar_index][0];
                   i_0 < tm.feed_shape0_range[feedvar_index][1];
                   ++i_0) {
                for (int i_1 = 0; i_1 < feedVarTensor.shape[1]; ++i_1) {
                  old_index = i_0 * feedVarTensor.shape[1] + i_1;
                  new_index = i_0 * feedvar_max_shape_vector[0] + i_1;
                  if (feedVarTensor.dtype == paddle::PaddleDType::INT64) {
                    *((int64_t*)dst_ptr + new_index) =
                        *((int64_t*)source_ptr + old_index);
                  } else if (feedVarTensor.dtype ==
                             paddle::PaddleDType::FLOAT32) {
                    *((float*)dst_ptr + new_index) =
                        *((float*)source_ptr + old_index);
                  } else if (feedVarTensor.dtype ==
                             paddle::PaddleDType::INT32) {
                    *((int32_t*)dst_ptr + new_index) =
                        *((int32_t*)source_ptr + old_index);
                  } else if (feedVarTensor.dtype ==
                             paddle::PaddleDType::UINT8) {
                    *((char*)dst_ptr + new_index) =
                        *((char*)source_ptr + old_index);
                  }
                }
              }
              break;
            default:
              break;
          }
        }

        // nobatch类型的feedvar，不叠加.
        if (tm.feedvar_type[feedvar_index] != 3) {
          _batch_in_offset[feedvar_index] += length;
        }
      }
    }
  }

  bool check_fetchvar_valid(size_t fetchvar_index) {
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

  size_t fetchvar_element_bytesize(size_t fetchvar_index) {
    if (!check_fetchvar_valid(fetchvar_index)) {
      return 0;
    }
    size_t dtype = _batch_out[fetchvar_index].dtype;
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
  size_t fetchvar_element_num(size_t fetchvar_index) {
    if (!check_fetchvar_valid(fetchvar_index)) {
      return 0;
    }
    size_t element_num = 1;
    if (_batch_out[fetchvar_index].shape.size() == 1) {
      // cause shape[0] is batch_size.
      return 1;
    }
    // start from shape[1], cause shape[0] = batch_size.
    for (size_t i = 1; i < _batch_out[fetchvar_index].shape.size(); ++i) {
      element_num *= _batch_out[fetchvar_index].shape[i];
    }
    return element_num;
  }

  size_t fetchvar_bytesize(size_t fetchvar_index) {
    return fetchvar_element_num(fetchvar_index) *
           fetchvar_element_bytesize(fetchvar_index);
  }

  size_t fetchvar_batch_size(size_t fetchvar_index) {
    if (!check_fetchvar_valid(fetchvar_index)) {
      return 0;
    }
    // if lod, 'lod[0].size()-1' is batch.
    // for PaddleTensor lod is vector<vector<size_t>>, so lod[0] is real lod.
    // for example, lod = [0,3,4,6], shape = [6,340,340], batch is 3 actually.
    // for lod, the batch < shape[0].
    if (_batch_out[fetchvar_index].lod.size() > 0 &&
        _batch_out[fetchvar_index].lod[0].size() > 0) {
      return _batch_out[fetchvar_index].lod[0].size() - 1;
    }
    // if not lod, the first dimension of data `PaddleTensor.shape[0]` is batch.
    return _batch_out[fetchvar_index].shape[0];
  }

  size_t fetchvar_batch_size() { return _total_fetch_batch; }

  bool deal_batch_out() {
    _total_fetch_batch = fetchvar_batch_size(0);
    if (_total_fetch_batch <= 0) return false;
    for (size_t fetchvar_index = 0; fetchvar_index < _batch_out.size();
         ++fetchvar_index) {
      // TODO(HexToString): Distinguish between nobatch and batch =
      // 1(By:HexToString)
      // 当数据中fetchvar-1: 带batch,且batch =1，shape[0] = 1
      // fetchvar-2:不带batch，由于不带batch导致shape[0] =1
      // 此时，无法分辨是否是天然nobatch，此时set_fetch_nobatch_index会漏掉
      // 后续希望在其他地方能够区分两者。
      if (fetchvar_batch_size(fetchvar_index) != _total_fetch_batch) {
        if (fetchvar_batch_size(fetchvar_index) <= 0) {
          // which means error.
          return false;
        } else if (fetchvar_batch_size(fetchvar_index) == 1) {
          // which means fetchvar shape[0] = 1.
          // shape[0] does not change with batch
          set_fetch_nobatch_index.insert(fetchvar_index);
        } else if (_total_fetch_batch == 1) {
          // 这时意味着，之前的fetchvar shape[0] 全部都= 1
          // 当前的fetchvar shape[0] > 1
          // 所以，之前的都是no_batch
          for (size_t temp_index = 0; temp_index < fetchvar_index;
               --temp_index) {
            set_fetch_nobatch_index.insert(fetchvar_index);
          }
          _total_fetch_batch = fetchvar_batch_size(fetchvar_index);
        } else {
          // which means error.
          return false;
        }
      }
      // 将lod fetchvar index加入到vector中。
      if (_batch_out[fetchvar_index].lod.size() > 0 &&
          _batch_out[fetchvar_index].lod[0].size() > 0) {
        vector_fetch_lod_index.push_back(fetchvar_index);
      }
    }
    return true;
  }

  void notify_tasks() {
    if (_taskmeta_vector.size() <= 0) {
      LOG(ERROR) << "_taskmeta_vector.size() <=0, error.";
      return;
    }
    // 根据_batch_out，求出输出的整体batch
    // 并将lod类型和nobatch类型的fetchvar的index记录到set中，方便后续查看。
    deal_batch_out();
    // 若输出的batch不是1，且不与输入batch对应，则错误
    if (_total_feed_batch != _total_fetch_batch && _total_fetch_batch != 1) {
      LOG(ERROR) << "_batch_out`s batch != _batch_in`s batch, error.";
      return;
    }

    size_t fetchvar_num = _batch_out.size();
    if (_batch_out_offset.size() == 0) {
      _batch_out_offset.resize(fetchvar_num, 0);
    }

    for (size_t ti = 0; ti < _taskmeta_vector.size(); ++ti) {
      TaskT* task = _taskmeta_vector[ti].task;
      size_t begin = _taskmeta_vector[ti].begin;
      size_t end = _taskmeta_vector[ti].end;
      size_t add = end - begin;
      size_t taskmeta_index = _taskmeta_vector[ti].taskmeta_index;
      // 对task中的outVectorT_ptr进行初始化
      // 如果是lod输出+多个taskmeta，此时对outLodTensorVector也需要初始化
      if (!task->task_fetch_init(*this)) {
        LOG(ERROR) << " task_fetch_init error.";
        return;
      }
      size_t fetch_lod_index = 0;

      for (size_t fetchvar_index = 0; fetchvar_index < fetchvar_num;
           ++fetchvar_index) {
        size_t fetchvar_bytesize_index = fetchvar_bytesize(fetchvar_index);
        if (set_fetch_nobatch_index.size() > 0 &&
            set_fetch_nobatch_index.find(fetchvar_index) !=
                set_fetch_nobatch_index.end()) {
          // nobatch fetchvar情况
          // 无论输入是多少batch，该index的fetchvar始终就shape[0] = 1
          paddle::PaddleTensor& fetchVarTensor =
              (*task->outVectorT_ptr)[fetchvar_index];
          void* dst_ptr = fetchVarTensor.data.data();
          size_t length = fetchvar_bytesize_index * 1;
          void* source_ptr = _batch_out[fetchvar_index].data.data();
          memcpy(dst_ptr, source_ptr, length);
        } else if (vector_fetch_lod_index.size() > 0 &&
                   std::find(vector_fetch_lod_index.begin(),
                             vector_fetch_lod_index.end(),
                             fetchvar_index) != vector_fetch_lod_index.end()) {
          // lod fetchvar情况，此时无法确定总的shape[0]
          // 根据task中的task_num总数开辟task_num个临时空间
          // 每个lod型的fetchvar拷贝到对应的临时空间中
          // 最后再计算临时空间的总量，合并fetchvar和lod
          size_t last_batch = _batch_out_offset[fetchvar_index];
          size_t shape0_index_start =
              _batch_out[fetchvar_index].lod[0][last_batch];
          size_t shape0_index_end =
              _batch_out[fetchvar_index].lod[0][last_batch + add];
          size_t shape0_length = shape0_index_end - shape0_index_start;
          size_t lod_size = _batch_out[fetchvar_index].lod.size();
          if (lod_size == 2) {
            shape0_index_start =
                _batch_out[fetchvar_index].lod[1][shape0_index_start];
            shape0_index_end =
                _batch_out[fetchvar_index].lod[1][shape0_index_end];
            shape0_length = shape0_index_end - shape0_index_start;
          }
          // task被拆分为多个taskmeta时，不能直接拷入task->outVectorT_ptr
          // 此时,先拷入task->outLodTensorVector[taskmeta_index]
          // 当task所有的taskmeta都完成时，再按照顺序进行拷贝回task->outVectorT_ptr。
          if (task->total_taskmeta_num > 1) {
            paddle::PaddleTensor& fetchVarTensor =
                task->outLodTensorVector[taskmeta_index][fetch_lod_index];
            size_t length = fetchvar_bytesize_index * shape0_length;
            fetchVarTensor.shape[0] = shape0_length;
            fetch_lod_index++;

            void* databuf_data =
                MempoolWrapper::instance().malloc(length, task->memoryPtr);
            paddle::PaddleBuf paddleBuf(databuf_data, length);
            fetchVarTensor.data = paddleBuf;
            // fetchVarTensor.data.Resize(length);
            void* dst_ptr = fetchVarTensor.data.data();
            void* source_ptr = _batch_out[fetchvar_index].data.data() +
                               shape0_index_start * fetchvar_bytesize_index;
            memcpy(dst_ptr, source_ptr, length);
            // 由于是拆分的各个lod，不要补0，在最后合并给Task中的outVectorT_ptr时再补。
            if (fetchVarTensor.lod.size() <= 0) {
              fetchVarTensor.lod.push_back({});
            }
            fetchVarTensor.lod[0].resize(add, 0);
            size_t last_lod_value =
                _batch_out[fetchvar_index].lod[0][last_batch];
            for (size_t lod_index = last_batch + 1, my_index = 0;
                 lod_index < last_batch + add + 1;
                 ++lod_index, ++my_index) {
              fetchVarTensor.lod[0][my_index] =
                  (_batch_out[fetchvar_index].lod[0][lod_index] -
                   last_lod_value);
            }
            if (lod_size == 2) {
              if (fetchVarTensor.lod.size() <= 1) {
                fetchVarTensor.lod.push_back({});
              }
              size_t last_lod0_value =
                  _batch_out[fetchvar_index].lod[0][last_batch];
              size_t end_lod0_value =
                  _batch_out[fetchvar_index].lod[0][last_batch + add];
              size_t lod1_size = end_lod0_value - last_lod0_value;
              fetchVarTensor.lod[1].resize(lod1_size, 0);
              for (size_t lod_index = last_lod0_value + 1, my_index = 0;
                   lod_index < end_lod0_value + 1;
                   ++lod_index, ++my_index) {
                fetchVarTensor.lod[1][my_index] =
                    _batch_out[fetchvar_index].lod[1][lod_index] -
                    _batch_out[fetchvar_index].lod[1][last_lod0_value];
              }
            }
          } else {
            // task未被拆分为多个taskmeta，故只有某个线程中的taskmeta会操作task不存在多线程竞争
            // 此时resize后，直接写入task->outVectorT_ptr中即可。
            paddle::PaddleTensor& fetchVarTensor =
                (*task->outVectorT_ptr)[fetchvar_index];
            size_t length = fetchvar_bytesize_index * shape0_length;
            fetchVarTensor.shape[0] = shape0_length;

            void* databuf_data =
                MempoolWrapper::instance().malloc(length, task->memoryPtr);
            paddle::PaddleBuf paddleBuf(databuf_data, length);
            fetchVarTensor.data = paddleBuf;

            // fetchVarTensor.data.Resize(length);
            void* dst_ptr = fetchVarTensor.data.data();
            void* source_ptr = _batch_out[fetchvar_index].data.data() +
                               shape0_index_start * fetchvar_bytesize_index;
            memcpy(dst_ptr, source_ptr, length);

            // task中的lod补0
            if (fetchVarTensor.lod.size() <= 0) {
              fetchVarTensor.lod.push_back({0});
            } else if (fetchVarTensor.lod[0].size() <= 0) {
              fetchVarTensor.lod[0].push_back(0);
            }
            // 将合并的lod信息对应的batch，拆分到task中。
            // 注意，此时需要去掉前面lod导致的前置积累。
            // 例如: 合lod = [0,2,5;7,10]，是由两组batch=2的task合并后预测的。
            // 此时拆分，第一组时，都减去0,得到[2,5]+(由于前面已经补了0了) =
            // [0,2,5]
            // 第二组，都需要减5,得到[2,5]，这样处理才对。
            fetchVarTensor.lod[0].resize(add + 1, 0);
            size_t last_lod_value =
                _batch_out[fetchvar_index].lod[0][last_batch];
            for (size_t lod_index = last_batch + 1, my_index = 1;
                 lod_index < last_batch + add + 1;
                 ++lod_index, ++my_index) {
              fetchVarTensor.lod[0][my_index] =
                  (_batch_out[fetchvar_index].lod[0][lod_index] -
                   last_lod_value);
            }

            if (lod_size == 2) {
              if (fetchVarTensor.lod.size() <= 1) {
                fetchVarTensor.lod.push_back({});
              } else if (fetchVarTensor.lod[1].size() <= 0) {
                fetchVarTensor.lod[1].push_back(0);
              }
              size_t last_lod0_value =
                  _batch_out[fetchvar_index].lod[0][last_batch];
              size_t end_lod0_value =
                  _batch_out[fetchvar_index].lod[0][last_batch + add];
              size_t lod1_size = end_lod0_value - last_lod0_value;
              fetchVarTensor.lod[1].resize(lod1_size + 1, 0);
              for (size_t lod_index = last_lod0_value + 1, my_index = 1;
                   lod_index < end_lod0_value + 1;
                   ++lod_index, ++my_index) {
                fetchVarTensor.lod[1][my_index] =
                    _batch_out[fetchvar_index].lod[1][lod_index] -
                    _batch_out[fetchvar_index].lod[1][last_lod0_value];
              }
            }
          }
        } else {
          // 普通fetchvar情况，此时该Task总的fetchvar_batch =
          // 输入的总的batch_size()
          // 输出的batch应与输入的batch对应相等。
          paddle::PaddleTensor& fetchVarTensor =
              (*task->outVectorT_ptr)[fetchvar_index];
          void* dst_ptr =
              fetchVarTensor.data.data() + fetchvar_bytesize_index * begin;
          size_t length = fetchvar_bytesize_index * add;
          void* source_ptr =
              _batch_out[fetchvar_index].data.data() +
              _batch_out_offset[fetchvar_index] * fetchvar_bytesize_index;

          memcpy(dst_ptr, source_ptr, length);
        }
        _batch_out_offset[fetchvar_index] += add;
      }

      // index是局部变量，fetch_add是原子操作，成功则返回原值。
      // 只有最后一个taskmeta都完成后，该线程的index+add才能>task->batch_size()
      // 故只有一个线程能进入if{}内.不会造成多线程竞争的问题。
      size_t index = task->index.fetch_add(add);
      if ((index + add) >= task->batch_size()) {
        task->combine_taskmeta();
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

  const size_t get_rem_size() { return _rem_size; }

  bool get_overrun() { return _overrun; }

  bool get_allow_split_request() { return _allow_split_request; }

 private:
  std::vector<TaskMetaT> _taskmeta_vector;
  typename TaskT::InVectorT _batch_in;
  std::vector<size_t> _batch_in_offset;
  std::vector<size_t> _total_shape0_batch_in;
  size_t _total_feed_batch;
  std::vector<PaddleTensorLod> _batch_in_lod;

  typename TaskT::OutVectorT _batch_out;
  std::vector<size_t> _batch_out_offset;
  // std::vector<size_t> _total_shape0_batch_out;
  size_t _total_fetch_batch;
  // std::vector<PaddleTensorLod>  _batch_out_lod;
  std::set<size_t> set_fetch_nobatch_index;
  std::vector<size_t> vector_fetch_lod_index;

  // 这个BatchTask中目前,各个FeedVar中最大的Shape集合

  size_t _rem_size;
  size_t _batch_size;
  bool _overrun;
  bool _allow_split_request;

  // 这个BatchTask中目前,各个FeedVar中最大的Shape集合
  VectorOfShapeVector vector_of_max_shape;
  // AutoPadding功能中，用这个与新的待合并Batch的TaskMeta来计算是否合并
  // 策略有两个，满足任何一个均可合并
  // 1、当相似度的乘积大于50%时
  // 2、当绝对差的字节数小于1024字节时
  // 例如，Shape-1 = [batch, 500, 500] Shape-2 = [batch, 400, 400]
  // 此时，绝对值差为90000字节，相对误差为0.8*0.8 = 0.64，满足条件1，不满足条件2
  // 例如，Shape-1 = [batch, 1, 1] Shape-2 = [batch, 2, 2]
  // 此时，绝对值差为3字节，相对误差为0.5*0.5 = 0.25，满足条件2，不满足条件1
  // 上述两种情况都可以进行AutoPadding.
  bool _auto_padding;
  int _padding_value;
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
  typedef std::vector<int> ShapeVector;
  typedef std::vector<ShapeVector> VectorOfShapeVector;

  TaskExecutor()
      : _stop(false),
        _thread_init_fn(NULL),
        _thread_reset_fn(NULL),
        _user_thread_contexts(NULL),
        _batch_size(DEFAULT_BATCH_SIZE),
        _overrun(false),
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

  void set_overrun(bool overrun) { _overrun = overrun; }

  void set_allow_split_request(bool allow_split_request) {
    _allow_split_request = allow_split_request;
  }

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

  TaskHandler<TaskT> schedule(const void*, void*, MempoolRegion* memoryPtr);

  bool move_task_to_batch(BatchTasks<TaskT>& batchTask);  // NOLINT

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
  bool _overrun;
  bool _allow_split_request;

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

  TaskExecutor<TaskT>& operator[](int task_index) {
    if (_vector_executor.size() <= task_index || task_index <= -1) {
      LOG(ERROR) << "_vector_executor.size() <= task_index or <= -1";
      throw "_vector_executor.size() <= task_index or <= -1";
    }
    return _vector_executor[task_index];
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

  explicit TaskManager(uint32_t model_index)  // NOLINT
      : _model_index(model_index) {}

  ~TaskManager() { wait(); }

  bool schedule(const void* in, void* out, MempoolRegion* memoryPtr);  // NOLINT
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
