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
using baidu::paddle_serving::predictor::general_model::Request;
using baidu::paddle_serving::predictor::general_model::FetchInst;
using baidu::paddle_serving::predictor::InferManager;
using baidu::paddle_serving::predictor::PaddleGeneralModelConfig;

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

  const Request *req = dynamic_cast<const Request *>(get_request_message());

  VLOG(2) << "start to call load general model_conf op";
  baidu::paddle_serving::predictor::Resource &resource =
      baidu::paddle_serving::predictor::Resource::instance();
  
  VLOG(2) << "get resource pointer done.";
  std::shared_ptr<PaddleGeneralModelConfig> model_config =
      resource.get_general_model_config();

  std::vector<int> fetch_index;
  fetch_index.resize(req->fetch_var_names_size());
  for (int i = 0; i < req->fetch_var_names_size(); ++i) {
    fetch_index[i] =
        model_config->_fetch_alias_name_to_index[req->fetch_var_names(i)];
  }
  
  // response inst with only fetch_var_names
  Response *res = mutable_data<Response>();

  for (int i = 0; i < batch_size; ++i) {
    FetchInst *fetch_inst = res->add_insts();
    for (auto & idx : fetch_index) {
      Tensor *tensor = fetch_inst->add_tensor_array();
      // currently only response float tensor or lod_tensor
      tensor->set_elem_type(1);
      if (model_config->_is_lod_fetch[idx]) {
        VLOG(2) << "out[" << idx << " is lod_tensor";
        tensor->add_shape(-1);
      } else {
        VLOG(2) << "out[" << idx << "] is tensor";
        for (int k = 1; k < out->at(idx).shape.size(); ++k) {
          VLOG(2) << "shape[" << k - 1 << "]: "
                  << out->at(idx).shape[k];
          tensor->add_shape(out->at(idx).shape[k]);
        }
      }
    }
  }

  int var_idx = 0;
  for (auto & idx : fetch_index) {
    float *data_ptr = static_cast<float *>(out->at(idx).data.data());
    int cap = 1;
    for (int j = 1; j < out->at(idx).shape.size(); ++j) {
      cap *= out->at(idx).shape[j];
    }
    if (model_config->_is_lod_fetch[idx]) {
      for (int j = 0; j < batch_size; ++j) {
        for (int k = out->at(idx).lod[0][j];
             k < out->at(idx).lod[0][j + 1]; k++) {
          res->mutable_insts(j)->mutable_tensor_array(var_idx)->add_data(
              reinterpret_cast<char *>(&(data_ptr[k])), sizeof(float));
        }
      }
    } else {
      for (int j = 0; j < batch_size; ++j) {
        for (int k = j * cap; k < (j + 1) * cap; ++k) {
          res->mutable_insts(j)->mutable_tensor_array(var_idx)->add_data(
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
