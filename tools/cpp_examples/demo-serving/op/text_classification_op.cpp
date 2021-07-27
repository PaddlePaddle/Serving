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

#include "examples/demo-serving/op/text_classification_op.h"
#include <algorithm>
#include "core/predictor/framework/infer.h"
#include "core/predictor/framework/memory.h"

namespace baidu {
namespace paddle_serving {
namespace serving {

using baidu::paddle_serving::predictor::MempoolWrapper;
using baidu::paddle_serving::predictor::text_classification::TextResInstance;
using baidu::paddle_serving::predictor::text_classification::Response;
using baidu::paddle_serving::predictor::text_classification::TextReqInstance;
using baidu::paddle_serving::predictor::text_classification::Request;

int inference_real(TextClassificationOp::TensorVector *in,
                   TextClassificationOp::TensorVector *out,
                   int sample_size) {
  if (predictor::InferManager::instance().infer(
          TEXT_CLASSIFICATION_MODEL_NAME, in, out, sample_size)) {
    LOG(ERROR) << "Failed do infer in fluid model: "
               << TEXT_CLASSIFICATION_MODEL_NAME;
    return -1;
  }

  return 0;
}

int inference_fake(TextClassificationOp::TensorVector *in,
                   TextClassificationOp::TensorVector *out,
                   int sample_size) {
  paddle::PaddleTensor lod_tensor;
  lod_tensor.dtype = paddle::PaddleDType::FLOAT32;
  lod_tensor.data.Resize(sample_size * sizeof(float) * 2);

  float *data_ptr = reinterpret_cast<float *>(lod_tensor.data.data());
  for (int i = 0; i < sample_size; ++i) {
    *(data_ptr + i * 2) = 0.500;
    *(data_ptr + i * 2 + 1) = 0.500;
  }

  lod_tensor.lod.resize(1);
  lod_tensor.lod[0].push_back(0);
  lod_tensor.lod[0].push_back(sample_size);

  lod_tensor.shape.push_back(sample_size);
  lod_tensor.shape.push_back(2);

  out->push_back(lod_tensor);
  return 0;
}

int TextClassificationOp::inference() {
  const Request *req = dynamic_cast<const Request *>(get_request_message());

  TensorVector *in = butil::get_object<TensorVector>();
  uint32_t sample_size = req->instances_size();
  if (sample_size <= 0) {
    LOG(WARNING) << "No instances need to inference!";
    return -1;
  }

  paddle::PaddleTensor lod_tensor;
  lod_tensor.dtype = paddle::PaddleDType::INT64;
  std::vector<std::vector<size_t>> &lod = lod_tensor.lod;
  lod.resize(1);
  lod[0].push_back(0);

  for (uint32_t si = 0; si < sample_size; ++si) {
    const TextReqInstance &req_instance = req->instances(si);
    lod[0].push_back(lod[0].back() + req_instance.ids_size());
  }

  lod_tensor.shape = {lod[0].back(), 1};
  lod_tensor.data.Resize(lod[0].back() * sizeof(int64_t));

  int offset = 0;
  for (uint32_t si = 0; si < sample_size; ++si) {
    // parse text sequence
    int64_t *data_ptr = static_cast<int64_t *>(lod_tensor.data.data()) + offset;
    const TextReqInstance &req_instance = req->instances(si);
    int id_count = req_instance.ids_size();
    memcpy(data_ptr,
           req_instance.ids().data(),
           sizeof(int64_t) * req_instance.ids_size());
    offset += req_instance.ids_size();
  }

  in->push_back(lod_tensor);

  TensorVector *out = butil::get_object<TensorVector>();
  if (!out) {
    LOG(ERROR) << "Failed get tls output object";
    return -1;
  }

  // call paddle fluid model for inferencing

  if (inference_real(in, out, sample_size) != 0) {
    return -1;
  }

  if (out->size() != in->size()) {
    LOG(ERROR) << "Output tensor size not equal that of input";
    return -1;
  }

  Response *res = mutable_data<Response>();

  for (size_t i = 0; i < out->size(); ++i) {
    int dim1 = out->at(i).shape[0];
    int dim2 = out->at(i).shape[1];

    if (out->at(i).dtype != paddle::PaddleDType::FLOAT32) {
      LOG(ERROR) << "Expected data type float";
      return -1;
    }

    float *data = static_cast<float *>(out->at(i).data.data());
    for (int j = 0; j < dim1; ++j) {
      TextResInstance *res_instance = res->add_predictions();
      res_instance->set_class_0_prob(data[j * dim2]);
      res_instance->set_class_1_prob(data[j * dim2 + 1]);
    }
  }

  for (size_t i = 0; i < in->size(); ++i) {
    (*in)[i].shape.clear();
  }
  in->clear();
  butil::return_object<TensorVector>(in);

  for (size_t i = 0; i < out->size(); ++i) {
    (*out)[i].shape.clear();
  }
  out->clear();
  butil::return_object<TensorVector>(out);
  return 0;
}

DEFINE_OP(TextClassificationOp);

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
