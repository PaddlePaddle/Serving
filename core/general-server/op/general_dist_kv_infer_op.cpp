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

#include "core/general-server/op/general_dist_kv_infer_op.h"
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
#include "core/util/include/timer.h"

namespace baidu {
namespace paddle_serving {
namespace serving {

using baidu::paddle_serving::Timer;
using baidu::paddle_serving::predictor::MempoolWrapper;
using baidu::paddle_serving::predictor::general_model::Tensor;
using baidu::paddle_serving::predictor::general_model::Response;
using baidu::paddle_serving::predictor::general_model::Request;
using baidu::paddle_serving::predictor::general_model::FetchInst;
using baidu::paddle_serving::predictor::InferManager;
using baidu::paddle_serving::predictor::PaddleGeneralModelConfig;

int GeneralDistKVInferOp::inference() { 
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
  if (!input_blob) {
    LOG(ERROR) << "input_blob is nullptr,error";
      return -1;
  }
  uint64_t log_id = input_blob->GetLogId();
  VLOG(2) << "(logid=" << log_id << ") Get precedent op name: " << pre_name;

  GeneralBlob *output_blob = mutable_data<GeneralBlob>();
  if (!output_blob) {
    LOG(ERROR) << "output_blob is nullptr,error";
      return -1;
  }
  output_blob->SetLogId(log_id);

  if (!input_blob) {
    LOG(ERROR) << "(logid=" << log_id
               << ") Failed mutable depended argument, op:" << pre_name;
    return -1;
  }
  Timer timeline;
  timeline.Start();
  const TensorVector *in = &input_blob->tensor_vector;
  TensorVector *out = &output_blob->tensor_vector;
  std::vector<uint64_t> keys;
  std::vector<uint64_t> rm_dup_keys;
  std::unordered_map<uint64_t, rec::mcube::CubeValue*> key_map;
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
  rm_dup_keys.resize(key_len);
  int key_idx = 0;
  for (size_t i = 0; i < dataptr_size_pairs.size(); ++i) {
    std::copy(dataptr_size_pairs[i].first,
              dataptr_size_pairs[i].first + dataptr_size_pairs[i].second,
              keys.begin() + key_idx);
    key_idx += dataptr_size_pairs[i].second;
  }

  int rm_dup_keys_count = 0;
  for (size_t i = 0; i < keys.size(); ++i) {
    if (key_map.find(keys[i]) == key_map.end()) {
       key_map[keys[i]] = nullptr;
       rm_dup_keys[rm_dup_keys_count++] = keys[i];
    }
  }
  rm_dup_keys.resize(rm_dup_keys_count);
  VLOG(2) << "(logid=" << log_id << ") cube number of keys to look up: " << key_len << " after rm dup keys: "<< rm_dup_keys_count;
  rec::mcube::CubeAPI *cube = rec::mcube::CubeAPI::instance();
  std::vector<std::string> table_names = cube->get_table_names();
  if (table_names.size() == 0) {
    LOG(ERROR) << "cube init error or cube config not given.";
    return -1;
  }
  int64_t seek_start = timeline.TimeStampUS();
  int ret = cube->seek(table_names[0], rm_dup_keys, &values);
  int64_t seek_end = timeline.TimeStampUS();
  VLOG(2) << "(logid=" << log_id << ") cube seek status: " << ret << " seek_time: " << seek_end - seek_start;
  for (size_t i = 0; i < rm_dup_keys.size(); ++i) {  
    key_map[rm_dup_keys[i]] = &values[i];   
  }
  if (values.size() != keys.size() || values[0].buff.size() == 0) {
    LOG(ERROR) << "cube value return null";
  }
  //size_t EMBEDDING_SIZE = values[0].buff.size() / sizeof(float);
  size_t EMBEDDING_SIZE = 9;
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
  std::shared_ptr<PaddleGeneralModelConfig> model_config = resource.get_general_model_config().front();
  int cube_key_found = 0;
  int cube_key_miss = 0; 
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
    sparse_out[sparse_idx].shape.push_back(sparse_out[sparse_idx].lod[0].back());
    sparse_out[sparse_idx].shape.push_back(EMBEDDING_SIZE);
    sparse_out[sparse_idx].name = model_config->_feed_name[i];
    sparse_out[sparse_idx].data.Resize(sparse_out[sparse_idx].lod[0].back() *
                                       EMBEDDING_SIZE * sizeof(float));
    float *dst_ptr = static_cast<float *>(sparse_out[sparse_idx].data.data());
    for (int x = 0; x < sparse_out[sparse_idx].lod[0].back(); ++x) {
      float *data_ptr = dst_ptr + x * EMBEDDING_SIZE;
      uint64_t cur_key = keys[cube_val_idx];
      rec::mcube::CubeValue* cur_val = key_map[cur_key];
      if (cur_val->buff.size() == 0) {
         memset(data_ptr, (float)0.0, sizeof(float) * EMBEDDING_SIZE);
         VLOG(3) << "(logid=" << log_id << ") cube key not found: " << keys[cube_val_idx];
         ++cube_key_miss;
	 ++cube_val_idx;
         continue;
      }
      //VLOG(3) << "(logid=" << log_id << ") cube key found: " << keys[cube_val_idx];
      memcpy(data_ptr, values[cube_val_idx].buff.data()+10, values[cube_val_idx].buff.size()-10);
      //VLOG(3) <<  keys[cube_val_idx] << ":" << data_ptr[0] << ", " << data_ptr[1] << ", " <<data_ptr[2] << ", " <<data_ptr[3] << ", " <<data_ptr[4] << ", " <<data_ptr[5] << ", " <<data_ptr[6] << ", " <<data_ptr[7] << ", " <<data_ptr[8];
      ++cube_key_found;
      ++cube_val_idx;
    }
    ++sparse_idx;
  }
  bool cube_fail = (cube_key_found == 0);
  if (cube_fail) {
    LOG(WARNING) << "(logid=" << log_id << ") cube seek fail";
    //CopyBlobInfo(input_blob, output_blob);
    //return 0;
  }
  VLOG(2) << "(logid=" << log_id << ") cube key found: " << cube_key_found << " , cube key miss: " << cube_key_miss;
  VLOG(2) << "(logid=" << log_id << ") sparse tensor load success.";
  timeline.Pause();
  VLOG(2) << "dist kv, cube and datacopy time: " << timeline.ElapsedUS();
  TensorVector infer_in;
  infer_in.insert(infer_in.end(), dense_out.begin(), dense_out.end());
  infer_in.insert(infer_in.end(), sparse_out.begin(), sparse_out.end());
  int batch_size = input_blob->_batch_size;
  output_blob->_batch_size = batch_size;
  int64_t start = timeline.TimeStampUS();
  timeline.Start();

  if (InferManager::instance().infer(
          engine_name().c_str(), &infer_in, out, batch_size)) {
    LOG(ERROR) << "Failed do infer in fluid model: " << engine_name();
    return -1;
  }
  int64_t end = timeline.TimeStampUS();
  if (cube_fail) { 
     float *out_ptr = static_cast<float*>(out->at(0).data.data());
     out_ptr[0] = 0.0;
  }
  timeline.Pause();
  VLOG(2) << "dist kv, pure paddle infer time: " << timeline.ElapsedUS(); 
  CopyBlobInfo(input_blob, output_blob);
  AddBlobInfo(output_blob, start);
  AddBlobInfo(output_blob, end);
  return 0; 

}
DEFINE_OP(GeneralDistKVInferOp);

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
