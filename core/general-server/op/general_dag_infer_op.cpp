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

  VLOG(2) << "begin to do infer in general dag infer op";
  GeneralBlob *output_blob = mutable_data<GeneralBlob>();

  VLOG(2) << "begin to get out";
  TensorVector *out = &output_blob->tensor_vector;

  VLOG(2) << "declare output blob done.";

  std::shared_ptr<PaddleGeneralModelConfig> config = get_config();
  int curr_op_idx = config->_graph.node_name_to_id[op_name()];

  VLOG(2) << "current op index: " << curr_op_idx;
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
  VLOG(2) << "output names size: " << output_names.size();
  VLOG(2) << "pre node names size: " << pre_node_names.size();
  VLOG(2) << "input name map size: " << input_name_map.size();
  TensorVector input;
  int batch_size = 0;
  const GeneralBlob *input_blob;
  for (int i = 0; i < pre_node_names.size(); ++i) {
    VLOG(2) << "pre node name[" << i << "]: "
            << pre_node_names[i];
    input_blob =
        get_depend_argument<GeneralBlob>(pre_node_names[i]);
    fprintf(stderr, "input blob address %x\n", input_blob);
    if (!input_blob) {
      LOG(ERROR) << "Failed mutable depended argument, op:" << pre_name();
      return -1;
    }
    batch_size = input_blob->GetBatchSize();
    VLOG(2) << "batch size of input: " << batch_size;
    for (int j = 0; j < input_blob->tensor_vector.size(); ++j) {
      VLOG(2) << "input tensor[" << j << "]: "
              << input_blob->tensor_vector[j].name;
      if (input_name_map.find(input_blob->tensor_vector[j].name)
          != input_name_map.end()) {
        // input.push_back(std::move(input_blob->tensor_vector[j]));
        // input.push_back(input_blob->tensor_vector[j]);
        paddle::PaddleTensor lod_tensor;
        lod_tensor.name = input_blob->tensor_vector[j].name;
        lod_tensor.lod.resize(1);
        lod_tensor.lod[0].push_back(0);
        lod_tensor.lod[0].push_back(1);
        lod_tensor.dtype = paddle::PaddleDType::INT64;
        lod_tensor.data.Resize(1 * sizeof(int64_t));
        lod_tensor.shape = {1, 1};
        int64_t * dst_ptr = static_cast<int64_t *>(lod_tensor.data.data());
        int64_t * src_ptr = static_cast<int64_t *>(
            input_blob->tensor_vector[j].data.data());
        dst_ptr[0] = src_ptr[0];
        input.push_back(lod_tensor);
        VLOG(2) << "add an input tensor name: " << lod_tensor.name;
      } else {
        VLOG(2) << "tensor name: " << input_blob->tensor_vector[j].name
                << " not found";
      }
    }
  }

  VLOG(2) << "get output blob done.";
  // const TensorVector *in = &input_blob->tensor_vector;
  const TensorVector *in = &input;
  VLOG(2) << "get input done.";

  batch_size = 1;
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
  CopyBlobInfo(input_blob, output_blob);
  AddBlobInfo(output_blob, start);
  AddBlobInfo(output_blob, end);
  return 0;
}
DEFINE_OP(GeneralDAGInferOp);

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
