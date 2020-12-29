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

#include "core/general-server/op/general_reader_op.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include "core/general-server/op/general_infer_helper.h"
#include "core/predictor/framework/infer.h"
#include "core/predictor/framework/memory.h"
#include "core/util/include/timer.h"

namespace baidu {
namespace paddle_serving {
namespace serving {

using baidu::paddle_serving::Timer;
using baidu::paddle_serving::predictor::MempoolWrapper;
using baidu::paddle_serving::predictor::general_model::Tensor;
using baidu::paddle_serving::predictor::general_model::Request;
using baidu::paddle_serving::predictor::general_model::FeedInst;
using baidu::paddle_serving::predictor::PaddleGeneralModelConfig;

int conf_check(const Request *req,
               const std::shared_ptr<PaddleGeneralModelConfig> &model_config) {
  int var_num = req->insts(0).tensor_array_size();
  if (var_num != model_config->_feed_type.size()) {
    LOG(ERROR) << "feed var number not match: model config["
               << model_config->_feed_type.size() << "] vs. actual[" << var_num
               << "]";
    return -1;
  }

  VLOG(2) << "fetch var num in reader op: " << req->fetch_var_names_size();

  for (int i = 0; i < var_num; ++i) {
    if (model_config->_feed_type[i] !=
        req->insts(0).tensor_array(i).elem_type()) {
      LOG(ERROR) << "feed type not match.";
      return -1;
    }
    if (model_config->_feed_shape[i].size() ==
        req->insts(0).tensor_array(i).shape_size()) {
      for (int j = 0; j < model_config->_feed_shape[i].size(); ++j) {
        req->insts(0).tensor_array(i).shape(j);
        if (model_config->_feed_shape[i][j] !=
            req->insts(0).tensor_array(i).shape(j)) {
          LOG(ERROR) << "feed shape not match.";
          return -1;
        }
      }
    } else {
      LOG(ERROR) << "feed shape not match.";
      return -1;
    }
  }
  return 0;
}

int GeneralReaderOp::inference() {
  // reade request from client
  return 0;
}
DEFINE_OP(GeneralReaderOp);
}  // namespace serving
}  // namespace paddle_serving
}  // namespace baidu
