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

#include "core/general-server/op/general_mod_op.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include "core/general-server/op/general_infer_helper.h"
#include "core/predictor/framework/infer.h"
#include "core/predictor/framework/memory.h"
#include "core/util/include/timer.h"

namespace baidu {
namespace paddle_serving {
namespace serving {

using baidu::paddle_serving::Timer;
using baidu::paddle_serving::predictor::MempoolWrapper;
using baidu::paddle_serving::predictor::general_model::Tensor;
using baidu::paddle_serving::predictor::general_model::Request;
using baidu::paddle_serving::predictor::general_model::FeedInst;
using baidu::paddle_serving::predictor::PaddleGeneralModelConfig;

int GeneralModOp::inference() {

  baidu::paddle_serving::predictor::Resource &resource =
      baidu::paddle_serving::predictor::Resource::instance();

  std::shared_ptr<PaddleGeneralModelConfig> config =
      resource.get_general_model_config();

  int curr_op_idx = config->_graph.node_name_to_id[op_name()];
  const std::vector<std::string> output_names =
      config->_graph.nodes[curr_op_idx].output_names;
  const std::vector<std::string> pre_node_names =
      config->_graph.nodes[curr_op_idx].pre_node_names;
  const std::vector<std::vector<int>> pre_input_idx =
      config->_graph.nodes[curr_op_idx].pre_input_idx;

  // this operator suppose every input tensor is a lodtensor with int64
  // the output of this op merge all the inputs into a single tensor vec
  // each item is mod by 100000001

  Timer timeline;
  int64_t start = timeline.TimeStampUS();

  GeneralBlob *output_blob = mutable_data<GeneralBlob>();
  TensorVector *out = &output_blob->tensor_vector;

  for (int i = 0; i < pre_node_names.size(); ++i) {
    const GeneralBlob *input_blob =
        get_depend_argument<GeneralBlob>(pre_node_names[i]);
    if (!input_blob) {
      LOG(ERROR) << "Failed mutable depended argument, op:" << pre_name();
      return -1;
    }
    for (int j = 0; j < pre_input_idx[i].size(); ++j) {
      out->push_back(input_blob->tensor_vector[pre_input_idx[i][j]]);
    }
  }

  const GeneralBlob *first_blob =
      get_depend_argument<GeneralBlob>(pre_node_names[0]);
  int batch_size = first_blob->GetBatchSize();
  output_blob->SetBatchSize(batch_size);

  int mod_value = 100000001;
  for (int i = 0; i < out->size(); ++i) {
    for (int j = 0; j < out->at(i).lod[0].back(); ++j) {
      out->at(i).data[j] = out->at(i).data[j] % mod_value;
    }
  }

  timeline.Pause();
  int64_t end = timeline.TimeStampUS();
  CopyBlobInfo(input_blob, output_blob);
  AddBlobInfo(output_blob, start);
  AddBlobInfo(output_blob, end);

  VLOG(2) << "read data from client success";
  return 0;
}

DEFINE_OP(GeneralModOp);
}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
