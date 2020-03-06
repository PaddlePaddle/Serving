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
#include "core/predictor/framework/infer.h"
#include "core/predictor/framework/memory.h"
#include "core/predictor/framework/resource.h"
#include "core/util/include/timer.h"
#include "core/cube/cube-api/include/cube_api.h"

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
  const GeneralBlob *input_blob = get_depend_argument<GeneralBlob>(pre_name());
  VLOG(2) << "Get precedent op name: " << pre_name();
  GeneralBlob *output_blob = mutable_data<GeneralBlob>();

  if (!input_blob) {
    LOG(ERROR) << "Failed mutable depended argument, op:" << pre_name();
    return -1;
  }

  const TensorVector *in = &input_blob->tensor_vector;
  TensorVector *out = &output_blob->tensor_vector;
  int batch_size = input_blob->GetBatchSize();
  VLOG(2) << "input batch size: " << batch_size;
  std::vector<uint64_t> keys;
  std::vector<rec::mcube::CubeValue> values;
  int sparse_count = 0;
  for (size_t i = 0; i < in->size(); ++i) {
    if (in->at(i).name == "dense_input") continue;
    ++sparse_count;
    size_t elem_num = 1;
    for (size_t s = 0; s < in->at(i).shape.size(); ++s) {
      elem_num *= in->at(i).shape[s];
    }
    int64_t* data_ptr = static_cast<int64_t*>(in->at(i).data.data());
    for (size_t j = 0; j < elem_num; ++j) {
      keys.push_back(data_ptr[j]);
    }
  }  
  //TODO: Add Seek CubeValues Here, and replace EMBEDDING_SIZE with variable.
  rec::mcube::CubeAPI *cube = rec::mcube::CubeAPI::instance();
  cube->init("./work_dir1/cube.conf");
  std::string table_name = "test_dict";
  int ret = cube->seek(table_name, keys, &values);
  //for (int vi = 0; vi < values.size(); ++vi) {
  //  std::cout << "value idx: " << vi << " , value: " << values[vi].buff << std::endl;
  //}
 
  size_t EMBEDDING_SIZE = 9; 
  TensorVector dist_kv_out;
  dist_kv_out.resize(sparse_count);
  int cube_val_idx = 0;
  int sparse_idx = 0;
  for (size_t i = 0; i < in->size(); ++i) {
    if  (in->at(i).name == "dense_input") continue;
    dist_kv_out[sparse_idx].lod.resize(in->at(i).lod.size());
    for (size_t x = 0; x < dist_kv_out[sparse_idx].lod.size(); ++x) {
      dist_kv_out[sparse_idx].lod[x].resize(in->at(i).lod[x].size());
      std::copy(in->at(i).lod[x].begin(), in->at(i).lod[x].end(), dist_kv_out[sparse_idx].lod[x].begin());
    }
    //std::cout << "sparse idx: " << sparse_idx << " , idx: " << i << std::endl;
    dist_kv_out[sparse_idx].dtype = paddle::PaddleDType::FLOAT32;
    dist_kv_out[sparse_idx].shape.push_back(dist_kv_out[sparse_idx].lod[0].back());
    dist_kv_out[sparse_idx].shape.push_back(EMBEDDING_SIZE); 
    dist_kv_out[sparse_idx].name =  "embedding_"+ std::to_string(sparse_idx)+ ".tmp_0";
    dist_kv_out[sparse_idx].data.Resize(dist_kv_out[sparse_idx].lod[0].back() * EMBEDDING_SIZE * sizeof(float));
    //std::cout << "embedding byte size: " << dist_kv_out[sparse_idx].lod[0].back() * EMBEDDING_SIZE * sizeof(float) << std::endl;
    float *dst_ptr = static_cast<float*>(dist_kv_out[sparse_idx].data.data());
    for (int x = 0; x < dist_kv_out[sparse_idx].lod[0].back(); ++x) {
      float * data_ptr = dst_ptr +  x* EMBEDDING_SIZE;
      //std::cout << "x: " << x << std::endl;
      memcpy(data_ptr, values[cube_val_idx].buff.data(), values[cube_val_idx].buff.size());
      cube_val_idx++;
    }
    ++sparse_idx;
  }  
  dist_kv_out.push_back(in->at(0));  
  output_blob->SetBatchSize(batch_size);

  VLOG(2) << "infer batch size: " << batch_size;

  Timer timeline;
  int64_t start = timeline.TimeStampUS();
  timeline.Start();

  if (InferManager::instance().infer(GENERAL_MODEL_NAME, &dist_kv_out, out, batch_size)) {
    LOG(ERROR) << "Failed do infer in fluid model: " << GENERAL_MODEL_NAME;
    return -1;
  }
  std::cout << "out size: " << out->size() << std::endl;
  for (size_t i = 0; i < out->size(); ++i) {
    float* res = static_cast<float*>(out->at(i).data.data());
    std::cout << "out tensor name: "<< out->at(i).name  << " , prob 0: " << res[0] << " , prob 1: " << res[1] << std::endl;
  }
  
  int64_t end = timeline.TimeStampUS();
  CopyBlobInfo(input_blob, output_blob);
  AddBlobInfo(output_blob, start);
  AddBlobInfo(output_blob, end);
  return 0;
}
DEFINE_OP(GeneralDistKVInferOp);

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
