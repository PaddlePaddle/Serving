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
#include <map>
#include <memory>
#include <sstream>
#include <utility>
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
using baidu::paddle_serving::predictor::general_model::ModelOutput;
using baidu::paddle_serving::predictor::InferManager;
using baidu::paddle_serving::predictor::PaddleGeneralModelConfig;

int GeneralResponseOp::inference() {
  const Request *req = dynamic_cast<const Request *>(get_request_message());
  // response inst with only fetch_var_names
  Response *res = mutable_data<Response>();
  baidu::paddle_serving::predictor::Resource &resource =
      baidu::paddle_serving::predictor::Resource::instance();
  std::shared_ptr<PaddleGeneralModelConfig> model_config =
      resource.get_general_model_config();
  std::vector<int> capacity(req->fetch_var_names_size(), 1);
  std::string engine_name = "general_infer_0";
  ModelOutput *output = res->add_outputs();
  FetchInst *fetch_inst = output->add_insts();
  FetchInst *fetch_p = output->mutable_insts(0);
  std::vector<std::string> outs =
      InferManager::instance().GetOutputNames(engine_name.c_str());
  for (int i = 0; i < req->fetch_var_names_size(); ++i) {
    Tensor *tensor = fetch_inst->add_tensor_array();
    std::string tensor_name = outs[i];
    auto lod_tensor = InferManager::instance().GetOutputHandle(
        engine_name.c_str(), tensor_name.c_str());
    std::vector<int> shape = lod_tensor->shape();
    for (int k = 0; k < shape.size(); ++k) {
      capacity[i] *= shape[k];
      tensor->add_shape(shape[k]);
    }
    auto dtype = lod_tensor->type();
    if (dtype == paddle::PaddleDType::INT64) {
      std::vector<int64_t> datas(capacity[i]);
      int64_t *data_ptr = datas.data();
      lod_tensor->CopyToCpu(data_ptr);
      google::protobuf::RepeatedField<int64_t> tmp_data(data_ptr,
                                                        data_ptr + capacity[i]);
      tensor->mutable_int64_data()->Swap(&tmp_data);
    } else if (dtype == paddle::PaddleDType::FLOAT32) {
      std::vector<float> datas(capacity[i]);
      float *data_ptr = datas.data();
      lod_tensor->CopyToCpu(data_ptr);
      google::protobuf::RepeatedField<float> tmp_data(data_ptr,
                                                      data_ptr + capacity[i]);
      tensor->mutable_float_data()->Swap(&tmp_data);
    } else if (dtype == paddle::PaddleDType::INT32) {
      std::vector<int32_t> datas(capacity[i]);
      int32_t *data_ptr = datas.data();
      lod_tensor->CopyToCpu(data_ptr);
      google::protobuf::RepeatedField<int32_t> tmp_data(data_ptr,
                                                        data_ptr + capacity[i]);
      tensor->mutable_int_data()->Swap(&tmp_data);
    }
    std::vector<std::vector<size_t>> lod = lod_tensor->lod();
    if (lod.size() > 0) {
      for (int j = 0; j < lod[0].size(); ++j) {
        tensor->add_lod(lod[0][j]);
      }
    }
  }
  return 0;
}

DEFINE_OP(GeneralResponseOp);

}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
