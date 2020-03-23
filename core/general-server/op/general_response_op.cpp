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

#include "core/general-server/op/general_response_op.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include "core/general-server/op/general_infer_helper.h"
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

int GeneralResponseOp::inference() {
  const GeneralBlob *input_blob = get_depend_argument<GeneralBlob>(pre_name());

  if (!input_blob) {
    LOG(ERROR) << "Failed mutable depended argument, op: " << pre_name();
    return -1;
  }

  const TensorVector *in = &input_blob->tensor_vector;
  int batch_size = input_blob->GetBatchSize();

  VLOG(2) << "input batch size: " << batch_size;

  const Request *req = dynamic_cast<const Request *>(get_request_message());

  Timer timeline;
  // double response_time = 0.0;
  // timeline.Start();
  int64_t start = timeline.TimeStampUS();

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
    for (auto &idx : fetch_index) {
      Tensor *tensor = fetch_inst->add_tensor_array();
      // currently only response float tensor or lod_tensor
      tensor->set_elem_type(1);
      if (model_config->_is_lod_fetch[idx]) {
        VLOG(2) << "out[" << idx << " is lod_tensor";
        tensor->add_shape(-1);
      } else {
        VLOG(2) << "out[" << idx << "] is tensor";
        for (int k = 1; k < in->at(idx).shape.size(); ++k) {
          VLOG(2) << "shape[" << k - 1 << "]: " << in->at(idx).shape[k];
          tensor->add_shape(in->at(idx).shape[k]);
        }
      }
    }
  }

  int var_idx = 0;
  for (auto &idx : fetch_index) {
    int cap = 1;
    for (int j = 1; j < in->at(idx).shape.size(); ++j) {
      cap *= in->at(idx).shape[j];
    }
    if (in->at(idx).dtype == paddle::PaddleDType::INT64) {
      int64_t *data_ptr = static_cast<int64_t *>(in->at(idx).data.data());
      if (model_config->_is_lod_fetch[idx]) {
        for (int j = 0; j < batch_size; ++j) {
          for (int k = in->at(idx).lod[0][j]; k < in->at(idx).lod[0][j + 1];
               k++) {
            FetchInst *fetch_p = res->mutable_insts(j);
            fetch_p->mutable_tensor_array(var_idx)->add_int64_data(data_ptr[k]);
          }
        }
      } else {
        int var_size = in->at(idx).shape[0];
        if (var_size == batch_size) {
          for (int j = 0; j < batch_size; ++j) {
            for (int k = j * cap; k < (j + 1) * cap; ++k) {
              FetchInst *fetch_p = res->mutable_insts(j);
              fetch_p->mutable_tensor_array(var_idx)->add_int64_data(
                  data_ptr[k]);
            }
          }
        } else {
          for (int j = 0; j < batch_size; ++j) {
            FetchInst *fetch_p = res->mutable_insts(j);
            fetch_p->mutable_tensor_array(var_idx)->add_int64_data(data_ptr[0]);
          }
        }
      }
      var_idx++;
    } else if (in->at(idx).dtype == paddle::PaddleDType::FLOAT32) {
      float *data_ptr = static_cast<float *>(in->at(idx).data.data());
      if (model_config->_is_lod_fetch[idx]) {
        for (int j = 0; j < batch_size; ++j) {
          for (int k = in->at(idx).lod[0][j]; k < in->at(idx).lod[0][j + 1];
               k++) {
            FetchInst *fetch_p = res->mutable_insts(j);
            fetch_p->mutable_tensor_array(var_idx)->add_float_data(data_ptr[k]);
          }
        }
      } else {
        int var_size = in->at(idx).shape[0];
        if (var_size == batch_size) {
          for (int j = 0; j < batch_size; ++j) {
            for (int k = j * cap; k < (j + 1) * cap; ++k) {
              FetchInst *fetch_p = res->mutable_insts(j);
              fetch_p->mutable_tensor_array(var_idx)->add_float_data(
                  data_ptr[k]);
            }
          }
        } else {
          for (int j = 0; j < batch_size; ++j) {
            FetchInst *fetch_p = res->mutable_insts(j);
            fetch_p->mutable_tensor_array(var_idx)->add_float_data(data_ptr[0]);
          }
        }
      }
      var_idx++;
    }
  }

  if (req->profile_server()) {
    int64_t end = timeline.TimeStampUS();
    VLOG(2) << "p size for input blob: " << input_blob->p_size;
    for (int i = 0; i < input_blob->p_size; ++i) {
      res->add_profile_time(input_blob->time_stamp[i]);
    }
    // TODO(guru4elephant): find more elegant way to do this
    res->add_profile_time(start);
    res->add_profile_time(end);
  }

  return 0;
}

DEFINE_OP(GeneralResponseOp);

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
