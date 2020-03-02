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

#include "core/general-server/op/general_dag_infer_op.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
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

int GeneralDAGInferOp::inference() {

  std::shared_ptr<PaddleGeneralModelConfig> config = get_config();

  int curr_op_idx = config->_graph.node_name_to_id[op_name()];
  const std::vector<std::string> output_names =
      config->_graph.nodes[curr_op_idx].output_names;

  const std::vector<std::string> pre_node_names =
      config->_graph.nodes[curr_op_idx].pre_node_names;

  const std::map<std::string, int> input_name_map =
      config->_graph.nodes[curr_op_idx].input_name_map;

  // here we suppose the input of inference can be from
  // multiple inputs, and some of the PaddleTensor can be
  // the input of inference
  // another assumption is that we suppose the input of the
  // inference has the same order as the input_blobs names
  // example:
  // GeneralBlob[0] = ["C_0", "C_1"], General_blob[1] = ["dense"]
  // Input Tensor = ["C_0", "C_1", "dense"]
  TensorVector input;
  for (int i = 0; i < pre_node_names.size(); ++i) {
    const GeneralBlob *input_blob =
        get_depend_argument<GeneralBlob>(pre_node_names[i]);
    if (!input_blob) {
      LOG(ERROR) << "Failed mutable depended argument, op:" << pre_name();
      return -1;
    }
    for (int j = 0; j < input_blob->tensor_vector.size(); ++j) {
      VLOG(2) << "input tensor[" << j << "]: "
              << input_blob->tensor_vector[j].name;
      if (input_name_map.find(input_blob->tensor_vector[j].name)
          != input_name_map.end()) {
        VLOG(2) << "added";
        input.push_back(std::move(input_blob->tensor_vector[j]));
      } else {
        VLOG(2) << "tensor name: " << input_blob->tensor_vector[j].name
                << " not found";
      }
    }
  }

  // we suppose at least one input blob
  // the batch size should be infer from this
  const GeneralBlob *first_blob =
      get_depend_argument<GeneralBlob>(pre_node_names[0]);
  int batch_size = first_blob->GetBatchSize();

  GeneralBlob *output_blob = mutable_data<GeneralBlob>();

  // const TensorVector *in = &input_blob->tensor_vector;
  const TensorVector *in = &input;
  TensorVector *out = &output_blob->tensor_vector;

  VLOG(2) << "input batch size: " << batch_size;
  output_blob->SetBatchSize(batch_size);

  Timer timeline;
  int64_t start = timeline.TimeStampUS();
  timeline.Start();

  VLOG(2) << "input of op " << op_name();
  for (int i = 0; i < in->size(); ++i) {
    VLOG(2) << in->at(i).name;
  }

  if (InferManager::instance().infer(GENERAL_MODEL_NAME, in, out, batch_size)) {
    LOG(ERROR) << "Failed do infer in fluid model: " << GENERAL_MODEL_NAME;
    return -1;
  }

  VLOG(2) << "output of op " << op_name();
  for (int i = 0; i < out->size(); ++i) {
    VLOG(2) << out->at(i).name;
  }

  int64_t end = timeline.TimeStampUS();
  CopyBlobInfo(first_blob, output_blob);
  AddBlobInfo(output_blob, start);
  AddBlobInfo(output_blob, end);
  return 0;
}
DEFINE_OP(GeneralDAGInferOp);

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
