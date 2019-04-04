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

#include "demo-serving/op/classify_op.h"
#include "demo-serving/op/reader_op.h"
#include "predictor/framework/infer.h"
#include "predictor/framework/memory.h"

namespace baidu {
namespace paddle_serving {
namespace serving {

using baidu::paddle_serving::predictor::format::DensePrediction;
using baidu::paddle_serving::predictor::image_classification::ClassifyResponse;
using baidu::paddle_serving::predictor::InferManager;

int ClassifyOp::inference() {
  const ReaderOutput* reader_out =
      get_depend_argument<ReaderOutput>("image_reader_op");
  if (!reader_out) {
    LOG(ERROR) << "Failed mutable depended argument, op:"
               << "reader_op";
    return -1;
  }

  const TensorVector* in = &reader_out->tensors;
  uint32_t sample_size = in->size();

  TensorVector* out = butil::get_object<TensorVector>();
  if (!out) {
    LOG(ERROR) << "Failed get tls output object failed";
    return -1;
  }

  if (sample_size <= 0) {
    LOG(INFO) << "No samples need to to predicted";
    return 0;
  }

  // call paddle fluid model for inferencing
  if (InferManager::instance().infer(
          IMAGE_CLASSIFICATION_MODEL_NAME, in, out, sample_size)) {
    LOG(ERROR) << "Failed do infer in fluid model: "
               << IMAGE_CLASSIFICATION_MODEL_NAME;
    return -1;
  }

  if (out->size() != sample_size) {
    LOG(ERROR) << "Output size not eq input size: " << in->size()
               << out->size();
    return -1;
  }

  // copy output tensor into response
  ClassifyResponse* res = mutable_data<ClassifyResponse>();
  for (uint32_t si = 0; si < sample_size; si++) {
    const paddle::PaddleTensor& out_tensor = (*out)[si];
    DensePrediction* ins = res->add_predictions();
    if (!ins) {
      LOG(ERROR) << "Failed append new out tensor";
      return -1;
    }

    uint32_t shape_size = out_tensor.shape.size();
    if (out_tensor.shape.size() != 2 || out_tensor.shape[0] != 1) {
      LOG(ERROR) << "Not valid classification out shape"
                 << ", shape size: " << out_tensor.shape.size();
      return -1;
    }

    // assign output data
    uint32_t data_size = out_tensor.data.length() / sizeof(float);
    float* data = reinterpret_cast<float*>(out_tensor.data.data());
    for (uint32_t di = 0; di < data_size; ++di) {
      ins->add_categories(data[di]);
    }
  }

  // release out tensor object resource
  size_t out_size = out->size();
  for (size_t oi = 0; oi < out_size; ++oi) {
    (*out)[oi].shape.clear();
  }
  out->clear();
  butil::return_object<TensorVector>(out);

  LOG(INFO) << "Response in image classification:"
            << "length:" << res->ByteSize() << ","
            << "data:" << res->ShortDebugString();

  return 0;
}

DEFINE_OP(ClassifyOp);

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
