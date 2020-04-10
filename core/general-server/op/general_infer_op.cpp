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

#include "core/general-server/op/general_infer_op.h"
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

int GeneralInferOp::inference() {
  VLOG(2) << "Going to run inference";
  //const GeneralBlob *input_blob = get_depend_argument<GeneralBlob>(pre_name());
  GeneralBlob *output_blob = mutable_data<GeneralBlob>();
  VLOG(2) << "finish get output_blob";
  TensorVector *out = &output_blob->tensor_vector;
  VLOG(2) << "finish get *out";
  
  const std::vector<std::string> pre_node_names = pre_names();
  VLOG(2) << "pre node names size: " << pre_node_names.size();
  TensorVector input;
  int batch_size = 0;
  const GeneralBlob *input_blob;
  for (int i = 0; i < (int)pre_node_names.size(); ++i) {
    VLOG(2) << "pre names[" << i << "]: "
            << pre_node_names[i];
    input_blob =
        get_depend_argument<GeneralBlob>(pre_node_names[i]);
    fprintf(stderr, "input blob address %x\n", input_blob);
    if (!input_blob) {
      LOG(ERROR) << "Failed mutable depended argument, op:" << pre_node_names[i];
      return -1;
    }
    batch_size = input_blob->GetBatchSize();
    VLOG(2) << "batch size of input: " << batch_size;
    for (int j = 0; j < input_blob->tensor_vector.size(); ++j) {
      VLOG(2) << "input tensor[" << j << "]: "
              << input_blob->tensor_vector[j].name;
      input.push_back(input_blob->tensor_vector[j]);
      VLOG(2) << "add an input tensor name: " << input_blob->tensor_vector[j].name;
    }
  }

  VLOG(2) << "get output blob done.";
  const TensorVector *in = &input;
  VLOG(2) << "get input done.";

  batch_size = 1;
  VLOG(2) << "infer batch size: " << batch_size;
  output_blob->SetBatchSize(batch_size);

  Timer timeline;
  int64_t start = timeline.TimeStampUS();
  timeline.Start();

  VLOG(2) << "input of op " << op_name();
  for (int i = 0; i < in->size(); ++i) {
    VLOG(2) << in->at(i).name;
  }

  VLOG(2) << "get engine name: " << engine_name().c_str();
  if (InferManager::instance().infer(
          GeneralInferOp::engine_name().c_str(), in, out, batch_size)) {
    LOG(ERROR) << "Failed do infer in fluid model: "
               << GeneralInferOp::engine_name();
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
DEFINE_OP(GeneralInferOp);

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
