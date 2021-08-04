// Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
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

#include "core/general-server/op/general_dist_kv_quant_infer_op.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <utility>
#include "core/cube/cube-api/include/cube_api.h"
#include "core/predictor/framework/infer.h"
#include "core/predictor/framework/memory.h"
#include "core/predictor/framework/resource.h"
#include "core/predictor/tools/quant.h"
#include "core/util/include/timer.h"

namespace baidu {
namespace paddle_serving {
namespace serving {

using baidu::paddle_serving::Timer;
using baidu::paddle_serving::predictor::MempoolWrapper;
using baidu::paddle_serving::predictor::general_model::Tensor;
using baidu::paddle_serving::predictor::general_model::Response;
using baidu::paddle_serving::predictor::general_model::Request;
using baidu::paddle_serving::predictor::InferManager;
using baidu::paddle_serving::predictor::PaddleGeneralModelConfig;

int GeneralDistKVQuantInferOp::inference() {
  VLOG(2) << "Going to run inference";
  const std::vector<std::string> pre_node_names = pre_names();
  if (pre_node_names.size() != 1) {
    LOG(ERROR) << "This op(" << op_name()
               << ") can only have one predecessor op, but received "
               << pre_node_names.size();
    return -1;
  }
  const std::string pre_name = pre_node_names[0];

  const GeneralBlob *input_blob = get_depend_argument<GeneralBlob>(pre_name);
  VLOG(2) << "Get precedent op name: " << pre_name;
  GeneralBlob *output_blob = mutable_data<GeneralBlob>();

  if (!input_blob) {
    LOG(ERROR) << "Failed mutable depended argument, op:" << pre_name;
    return -1;
  }

  uint64_t log_id = input_blob->GetLogId();
  output_blob->SetLogId(log_id);

  const TensorVector *in = &input_blob->tensor_vector;
  TensorVector *out = &output_blob->tensor_vector;
  int batch_size = input_blob->GetBatchSize();
  VLOG(2) << "(logid=" << log_id << ") input batch size: " << batch_size;
  std::vector<uint64_t> keys;
  std::vector<rec::mcube::CubeValue> values;
  int sparse_count = 0;
  int dense_count = 0;
  std::vector<std::pair<int64_t *, size_t>> dataptr_size_pairs;
  size_t key_len = 0;
  for (size_t i = 0; i < in->size(); ++i) {
    if (in->at(i).dtype != paddle::PaddleDType::INT64) {
      ++dense_count;
      continue;
    }
    ++sparse_count;
    size_t elem_num = 1;
    for (size_t s = 0; s < in->at(i).shape.size(); ++s) {
      elem_num *= in->at(i).shape[s];
    }
    key_len += elem_num;
    int64_t *data_ptr = static_cast<int64_t *>(in->at(i).data.data());
    dataptr_size_pairs.push_back(std::make_pair(data_ptr, elem_num));
  }
  keys.resize(key_len);
  int key_idx = 0;
  for (size_t i = 0; i < dataptr_size_pairs.size(); ++i) {
    std::copy(dataptr_size_pairs[i].first,
              dataptr_size_pairs[i].first + dataptr_size_pairs[i].second,
              keys.begin() + key_idx);
    key_idx += dataptr_size_pairs[i].second;
  }
  rec::mcube::CubeAPI *cube = rec::mcube::CubeAPI::instance();
  std::vector<std::string> table_names = cube->get_table_names();
  if (table_names.size() == 0) {
    LOG(ERROR) << "(logid=" << log_id
               << ") cube init error or cube config not given.";
    return -1;
  }
  int ret = cube->seek(table_names[0], keys, &values);

  if (values.size() != keys.size() || values[0].buff.size() == 0) {
    LOG(ERROR) << "(logid=" << log_id << ") cube value return null";
  }

  TensorVector sparse_out;
  sparse_out.resize(sparse_count);
  TensorVector dense_out;
  dense_out.resize(dense_count);
  int cube_val_idx = 0;
  int sparse_idx = 0;
  int dense_idx = 0;
  std::unordered_map<int, int> in_out_map;
  baidu::paddle_serving::predictor::Resource &resource =
      baidu::paddle_serving::predictor::Resource::instance();
  int cube_quant_bits = resource.get_cube_quant_bits();
  size_t EMBEDDING_SIZE = 0;
  if (cube_quant_bits == 0) {
    EMBEDDING_SIZE = values[0].buff.size() / sizeof(float);
  } else {
    EMBEDDING_SIZE = values[0].buff.size() - 2 * sizeof(float);
  }

  for (size_t i = 0; i < in->size(); ++i) {
    if (in->at(i).dtype != paddle::PaddleDType::INT64) {
      dense_out[dense_idx] = in->at(i);
      ++dense_idx;
      continue;
    }

    sparse_out[sparse_idx].lod.resize(in->at(i).lod.size());
    for (size_t x = 0; x < sparse_out[sparse_idx].lod.size(); ++x) {
      sparse_out[sparse_idx].lod[x].resize(in->at(i).lod[x].size());
      std::copy(in->at(i).lod[x].begin(),
                in->at(i).lod[x].end(),
                sparse_out[sparse_idx].lod[x].begin());
    }
    sparse_out[sparse_idx].dtype = paddle::PaddleDType::FLOAT32;
    sparse_out[sparse_idx].shape.push_back(
        sparse_out[sparse_idx].lod[0].back());
    sparse_out[sparse_idx].shape.push_back(EMBEDDING_SIZE);
    sparse_out[sparse_idx].name = in->at(i).name;
    sparse_out[sparse_idx].data.Resize(sparse_out[sparse_idx].lod[0].back() *
                                       EMBEDDING_SIZE * sizeof(float));
    // END HERE
    float *dst_ptr = static_cast<float *>(sparse_out[sparse_idx].data.data());
    for (int x = 0; x < sparse_out[sparse_idx].lod[0].back(); ++x) {
      float *data_ptr = dst_ptr + x * EMBEDDING_SIZE;
      if (cube_quant_bits == 0) {
        memcpy(data_ptr,
               values[cube_val_idx].buff.data(),
               values[cube_val_idx].buff.size());
      } else {
        // min (float), max (float), num, num, num... (Byte)
        size_t num_of_float =
            values[cube_val_idx].buff.size() - 2 * sizeof(float);
        float *float_ptr = new float[num_of_float];
        char *src_ptr = new char[values[cube_val_idx].buff.size()];
        memcpy(src_ptr,
               values[cube_val_idx].buff.data(),
               values[cube_val_idx].buff.size());
        float *minmax = reinterpret_cast<float *>(src_ptr);
        dequant(src_ptr + 2 * sizeof(float),
                float_ptr,
                minmax[0],
                minmax[1],
                num_of_float,
                cube_quant_bits);
        memcpy(data_ptr, float_ptr, sizeof(float) * num_of_float);
        delete float_ptr;
        delete src_ptr;
      }
      cube_val_idx++;
    }
    ++sparse_idx;
  }
  TensorVector infer_in;
  infer_in.insert(infer_in.end(), dense_out.begin(), dense_out.end());
  infer_in.insert(infer_in.end(), sparse_out.begin(), sparse_out.end());

  output_blob->SetBatchSize(batch_size);

  VLOG(2) << "(logid=" << log_id << ") infer batch size: " << batch_size;

  return 0;
}
DEFINE_OP(GeneralDistKVQuantInferOp);

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
