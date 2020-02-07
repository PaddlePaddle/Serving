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
#include "core/general-server/op/general_reader_op.h"
#include "core/predictor/framework/infer.h"
#include "core/predictor/framework/memory.h"
#include "core/predictor/framework/resource.h"


namespace baidu {
namespace paddle_serving {
namespace serving {

using baidu::paddle_serving::predictor::MempoolWrapper;
using baidu::paddle_serving::predictor::general_model::Tensor;
using baidu::paddle_serving::predictor::general_model::Response;
using baidu::paddle_serving::predictor::general_model::FetchInst;
using baidu::paddle_serving::predictor::InferManager;

int GeneralInferOp::inference() {
  const GeneralReaderOutput *reader_out =
      get_depend_argument<GeneralReaderOutput>("general_reader_op");
  if (!reader_out) {
    LOG(ERROR) << "Failed mutable depended argument, op:"
               << "general_reader_op";
    return -1;
  }

  int reader_status = reader_out->reader_status;
  if (reader_status != 0) {
    LOG(ERROR) << "Read request wrong.";
    return -1;
  }

  const TensorVector *in = &reader_out->tensor_vector;
  TensorVector *out = butil::get_object<TensorVector>();
  int batch_size = (*in)[0].shape[0];
  // infer
  if (InferManager::instance().infer(GENERAL_MODEL_NAME, in, out, batch_size)) {
    LOG(ERROR) << "Failed do infer in fluid model: " << GENERAL_MODEL_NAME;
    return -1;
  }

  Response *res = mutable_data<Response>();

  for (int i = 0; i < batch_size; ++i) {
    FetchInst *fetch_inst = res->add_insts();
    for (int j = 0; j < out->size(); ++j) {
      Tensor *tensor = fetch_inst->add_tensor_array();
      tensor->set_elem_type(1);
      if (out->at(j).lod.size() == 1) {
        tensor->add_shape(-1);
      } else {
        for (int k = 1; k < out->at(j).shape.size(); ++k) {
          tensor->add_shape(out->at(j).shape[k]);
        }
      }
    }
  }

  for (int i = 0; i < out->size(); ++i) {
    float *data_ptr = static_cast<float *>(out->at(i).data.data());
    int cap = 1;
    for (int j = 1; j < out->at(i).shape.size(); ++j) {
      cap *= out->at(i).shape[j];
    }
    if (out->at(i).lod.size() == 1) {
      for (int j = 0; j < batch_size; ++j) {
        for (int k = out->at(i).lod[0][j]; k < out->at(i).lod[0][j + 1]; k++) {
          res->mutable_insts(j)->mutable_tensor_array(i)->add_data(
              reinterpret_cast<char *>(&(data_ptr[k])), sizeof(float));
        }
      }
    } else {
      for (int j = 0; j < batch_size; ++j) {
        for (int k = j * cap; k < (j + 1) * cap; ++k) {
          res->mutable_insts(j)->mutable_tensor_array(i)->add_data(
              reinterpret_cast<char *>(&(data_ptr[k])), sizeof(float));
        }
      }
    }
  }
  return 0;
}
DEFINE_OP(GeneralInferOp);

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
