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

#include "core/general-server/op/general_dag_mod_op.h"
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

int GeneralDAGModOp::inference() {
  std::shared_ptr<PaddleGeneralModelConfig> config = get_config();

  int curr_op_idx = config->_graph.node_name_to_id[op_name()];
  VLOG(2) << "current op idx: " << curr_op_idx;
  const std::vector<std::string> output_names =
      config->_graph.nodes[curr_op_idx].output_names;
  const std::vector<std::string> pre_node_names =
      config->_graph.nodes[curr_op_idx].pre_node_names;
  const std::map<std::string, int> input_name_map =
      config->_graph.nodes[curr_op_idx].input_name_map;

  // this operator suppose every input tensor is a lodtensor with int64
  // the output of this op merge all the inputs into a single tensor vec
  // each item is mod by 100000001

  Timer timeline;
  int64_t start = timeline.TimeStampUS();

  VLOG(2) << "going to declare output blob";
  GeneralBlob *output_blob = mutable_data<GeneralBlob>();
  TensorVector *out = &output_blob->tensor_vector;

  VLOG(2) << "pre node names size: " << pre_node_names.size();
  int output_idx = 0;
  for (int i = 0; i < pre_node_names.size(); ++i) {
    VLOG(2) << "pre_node_names[" << i << "]: " << pre_node_names[i];
    const GeneralBlob *input_blob =
        get_depend_argument<GeneralBlob>(pre_node_names[i]);
    if (!input_blob) {
      LOG(ERROR) << "Failed mutable depended argument, op:" << pre_name();
      return -1;
    }
    for (int j = 0; j < input_blob->tensor_vector.size(); ++j) {
      if (input_name_map.find(input_blob->tensor_vector[j].name)
          != input_name_map.end()) {
        out->push_back(input_blob->tensor_vector[j]);
        out->back().name = output_names[output_idx++];
      }
    }
  }

  VLOG(2) << "push back input dense done.";
  VLOG(2) << "pre node names[0]: " << pre_node_names[0];
  const GeneralBlob *first_blob =
      get_depend_argument<GeneralBlob>(pre_node_names[0]);
  int batch_size = first_blob->GetBatchSize();
  VLOG(2) << "batch size of the first blob: " << batch_size;

  VLOG(2) << "batch size of this output: " << batch_size;
  output_blob->SetBatchSize(batch_size);

  int mod_value = 100000001;
  VLOG(2) << "output tensor size: " << out->size();
  for (int i = 0; i < out->size(); ++i) {
    int64_t *dst_ptr = static_cast<int64_t *>(out->at(i).data.data());
    if (out->at(i).lod.size() == 1) {
      VLOG(2) << "lod back size: " << out->at(i).lod[0].back();
    }
    for (int j = 0; j < out->at(i).lod[0].back(); ++j) {
      dst_ptr[j] = dst_ptr[j] % mod_value;
    }
  }

  VLOG(2) << "compute done.";
  timeline.Pause();
  int64_t end = timeline.TimeStampUS();
  CopyBlobInfo(first_blob, output_blob);
  AddBlobInfo(output_blob, start);
  AddBlobInfo(output_blob, end);

  VLOG(2) << "general mod op done.";
  return 0;
}

DEFINE_OP(GeneralDAGModOp);
}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
