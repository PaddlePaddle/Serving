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

#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include "core/general-server/op/general_infer_op.h"
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
  const GeneralBlob * input_blob =
      get_depend_argument<GeneralBlob>(pre_name());

  GeneralBlob * output_blob = mutable_data<GeneralBlob>();

  if (!input_blob) {
    LOG(ERROR) << "Failed mutable depended argument, op:"
               << pre_name();
    return -1;
  }

  const TensorVector *in = &input_blob->tensor_vector;
  TensorVector *out = &output_blob->tensor_vector;
  int batch_size = input_blob->GetBatchSize();

  VLOG(2) << "infer batch size: " << batch_size;
  // infer
  Timer timeline;
  double infer_time = 0.0;
  timeline.Start();
  if (InferManager::instance().infer(GENERAL_MODEL_NAME, in, out, batch_size)) {
    LOG(ERROR) << "Failed do infer in fluid model: " << GENERAL_MODEL_NAME;
    return -1;
  }
  timeline.Pause();
  infer_time = timeline.ElapsedUS();

  return 0;
}
DEFINE_OP(GeneralInferOp);

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
